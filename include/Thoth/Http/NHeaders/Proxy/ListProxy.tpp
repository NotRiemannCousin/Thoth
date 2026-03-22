#pragma once
#include <print>
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <Thoth/Utils/Functional.hpp>

namespace Thoth::Http::NHeaders {
    template<Serializable T, bool IsConst>
    ListProxy<T, IsConst>::ListProxy(string_view key, HeaderType &headers) : key{ key }, headers{ headers } { }

    template<Serializable T, bool IsConst>
    std::expected<typename ListProxy<T, IsConst>::Ts, InvalidHeaderFormat> ListProxy<T, IsConst>::GetWithDefault(Ts defaultValue) && {
        auto val{ headers.Get(key) };

        if (!val) return defaultValue;

        if (auto parsed{ ParseList(*val) }; !parsed)
            return *parsed;
        return std::unexpected{ InvalidHeaderFormat{} };    }

    template<Serializable T, bool IsConst>
    std::optional<typename ListProxy<T, IsConst>::Ts> ListProxy<T, IsConst>::GetAsOpt() && {
        return headers.Get(key).and_then(&ParseList);
    }

    template<Serializable T, bool IsConst>
    std::expected<typename ListProxy<T, IsConst>::Ts, HeaderErrorEnum> ListProxy<T, IsConst>::Get() && {
        const auto val{ headers.Get(key) };

        if (!val) return std::unexpected{ HeaderErrorEnum::NotFound };
        if ((*val)->empty()) return std::unexpected{ HeaderErrorEnum::EmptyValue };

        if (auto parsed{ val.and_then(&ParseList) }; parsed)
            return *parsed;
        return std::unexpected{ HeaderErrorEnum::InvalidFormat };
    }

    template<Serializable T, bool IsConst> // NOLINT(*-unconventional-assign-operator)
    template<std::ranges::range R>
        requires (!IsConst)
    void ListProxy<T, IsConst>::operator=(R &&newValue) && {
        Set(std::forward<R>(newValue));
    }

    template<Serializable T, bool IsConst>
    template<std::ranges::range R>
        requires (!IsConst)
    void ListProxy<T, IsConst>::Set(R &&newValue) && {
        static constexpr auto s_format = [](auto& obj) {
            return std::format("{}", obj);
        };
        headers.Set(key, newValue
                | std::views::transform(s_format)
                | std::views::join_with(',')
                | std::ranges::to<string>());
    }

    template<Serializable T, bool IsConst>
    template<class>
        requires (!IsConst)
    void ListProxy<T, IsConst>::Add(const T &newItem) && {
        headers.Add(newItem);
    }

    template<Serializable T, bool IsConst>
    template<class>
        requires (!IsConst)
    bool ListProxy<T, IsConst>::TrySet(std::string_view newValue) && {
        auto val{ Scan<T>(newValue) };
        if (!val) return false;

        Set(val);
        return true;
    }

    template<Serializable T, bool IsConst>
    std::optional<typename ListProxy<T, IsConst>::Ts> ListProxy<T, IsConst>::ParseList(HeaderValue* val) {
        // auto res{ Utils::FoldWhileSuccess(*val
        //         | std::views::split(',')
        //         | std::views::transform([](auto subrange){ return string_view{ subrange.begin(), subrange.end() }; })
        //         | std::views::transform(Scan<T>), Utils::SelfHof(&Ts::push_back), Ts{})
        // };
        // if (res) return res;
        // return std::nullopt;

        Ts res{};
        for (auto member : *val
                | std::views::split(',')
                | std::views::transform([](auto subrange){ return string_view{ subrange.begin(), subrange.end() }; })
                | std::views::transform(Scan<T>)) {

            if (!member)
                return std::nullopt;
            res.push_back(*member);
        }
        return res;
    }
}


#pragma once
#include <print>
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <Thoth/Utils/Functional.hpp>

namespace Thoth::Http::NHeaders {
    template<Serializable T>
    ListProxy<T>::ListProxy(string_view key, Headers &headers) : key{ key }, headers{ headers } { }

    template<Serializable T>
    std::expected<typename ListProxy<T>::Ts, InvalidHeaderFormat> ListProxy<T>::GetWithDefault(Ts defaultValue) && {
        auto val{ Get() };
        if (val) return *val;
        if (val.error() != HeaderErrorEnum::InvalidFormat) return defaultValue;
        return std::unexpected{ InvalidHeaderFormat{} };
    }

    template<Serializable T>
    std::optional<typename ListProxy<T>::Ts> ListProxy<T>::GetAsOpt() && {
        return headers.Get(key).and_then(&ParseList);
    }

    template<Serializable T>
    std::expected<typename ListProxy<T>::Ts, HeaderErrorEnum> ListProxy<T>::Get() && {
        const auto val{ headers.Get(key) };

        if (!val) return std::unexpected{ HeaderErrorEnum::NotFound };
        if ((*val)->empty()) return std::unexpected{ HeaderErrorEnum::EmptyValue };

        if (auto parsed{ val.and_then(&ParseList) }; parsed)
            return *parsed;
        return std::unexpected{ HeaderErrorEnum::InvalidFormat };
    }

    template<Serializable T>
    template<std::ranges::range R>
    void ListProxy<T>::operator=(R &&newValue) && {
        Set(std::forward<R>(newValue));
    }

    template<Serializable T>
    template<std::ranges::range R>
    void ListProxy<T>::Set(R &&newValue) && {
        static constexpr auto s_format = [](auto& obj) {
            return std::format("{}", obj);
        };
        headers.Set(key, newValue
                | std::views::transform(s_format)
                | std::views::join_with(',')
                | std::ranges::to<string>());
    }

    template<Serializable T>
    void ListProxy<T>::Add(const T &newItem) && {
        headers.Add(newItem);
    }

    template<Serializable T>
    bool ListProxy<T>::TrySet(std::string_view newValue) && {
        auto val{ Scan<T>(newValue) };
        if (!val) return false;

        Set(val);
        return true;
    }

    template<Serializable T>
    std::optional<typename ListProxy<T>::Ts> ListProxy<T>::ParseList(HeaderValue* val) {
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


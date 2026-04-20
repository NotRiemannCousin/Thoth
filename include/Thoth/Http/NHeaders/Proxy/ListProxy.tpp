#pragma once
#include <print>
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <Thoth/Utils/Functional.hpp>

namespace Thoth::Http::NHeaders {

    template<bool IsConst, Utils::Serializable ...Ts>
    ListProxy<IsConst, Ts...>::ListProxy(std::string_view key, HeaderType& headers, PatternType inPattern)
            : key{ key }, headers{ headers }, inPattern{ inPattern } { }

    template<bool IsConst, Utils::Serializable ...Ts>
    auto ListProxy<IsConst, Ts...>::GetAsOpt() && -> std::optional<Type> {
        auto val{ headers.Get(key) };

        if (!val || (*val)->empty()) return std::nullopt;

        if constexpr (Single) {
            return ParseList<Ts...[0]>(*val, inPattern);
        } else {
            std::optional<Type> result;
            std::size_t i{};
            ([&] {
                if (auto parsed{ ParseList<Ts>(*val, inPattern[i++]) }; parsed) {
                    result = Type{ std::in_place_type<std::vector<Ts>>, std::move(*parsed) };
                    return true;
                }
                return true;
            }() || ...);
            if (result) return result;
        }
        return std::nullopt;
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    auto ListProxy<IsConst, Ts...>::Get() && -> std::expected<Type, HeaderErrorEnum> {
        auto val{ headers.Get(key) };

        if (!val)            return std::unexpected{ HeaderErrorEnum::NotFound };
        if ((*val)->empty()) return std::unexpected{ HeaderErrorEnum::EmptyValue };

        if constexpr (Single) {
            if (auto parsed{ ParseList<Ts...[0]>(*val, inPattern) }; parsed)
                return std::move(*parsed);
        } else {
            std::optional<Type> result;
            std::size_t i{};
            ([&] {
                if (auto parsed{ ParseList<Ts>(*val, inPattern[i++]) }; parsed) {
                    result = Type{ std::in_place_type<std::vector<Ts>>, std::move(*parsed) };
                    return true;
                }
                return true;
            }() || ...);
            if (result) return std::move(*result);
        }

        return std::unexpected{ HeaderErrorEnum::InvalidFormat };
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    auto ListProxy<IsConst, Ts...>::GetWithDefault(Type defaultValue) && -> std::expected<Type, InvalidHeaderFormat> {
        auto val{ headers.Get(key) };

        if (!val || (*val)->empty()) return defaultValue;

        if constexpr (Single) {
            if (auto parsed{ ParseList<Ts...[0]>(*val, inPattern) }; parsed)
                return std::move(*parsed);
        } else {
            std::optional<Type> result;
            std::size_t i{};
            ([&] {
                if (auto parsed{ ParseList<Ts>(*val, inPattern[i++]) }; parsed) {
                    result = Type{ std::in_place_type<std::vector<Ts>>, std::move(*parsed) };
                    return true;
                }
                return true;
            }() || ...);
            if (result) return result;
            if (result) return std::move(*result);
        }

        return std::unexpected{ InvalidHeaderFormat{} };
    }


    template<bool IsConst, Utils::Serializable ...Ts> // NOLINT(*-unconventional-assign-operator)
    template<std::ranges::range R>
        requires (!IsConst)
    void ListProxy<IsConst, Ts...>::operator=(R&& newValue) && {
        std::move(*this).Set(std::forward<R>(newValue));
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    template<std::ranges::range R>
        requires (!IsConst)
    void ListProxy<IsConst, Ts...>::Set(R&& newValue) && {
        static constexpr auto s_format = [](auto& obj) {
            return std::format("{}", obj);
        };
        headers.Set(key, newValue
            | std::views::transform(s_format)
            | std::views::join_with(',')
            | std::ranges::to<std::string>());
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    template<class>
        requires (!IsConst)
    void ListProxy<IsConst, Ts...>::Add(const ElemType& newItem) && {
        if constexpr (Single) {
            headers.Add(key, std::format("{}", newItem));
        } else {
            std::visit([&](const auto& item) {
                headers.Add(key, std::format("{}", item));
            }, newItem);
        }
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    template<class>
        requires (!IsConst)
    bool ListProxy<IsConst, Ts...>::TrySet(std::string_view newValue) && {
        HeaderValue temp{ newValue };
        if constexpr (Single) {
            auto parsed{ ParseList<Ts...[0]>(&temp, inPattern) };
            if (!parsed) return false;
            std::move(*this).Set(std::move(*parsed));
            return true;
        } else {
            bool set{};
            std::size_t i{};
            ([&] {
                if (!set)
                    if (auto parsed{ ParseList<Ts>(&temp, inPattern[i]) }; parsed) {
                        std::move(*this).Set(std::move(*parsed));
                        set = true;
                    }
                ++i;
            }(), ...);
            return set;
        }
    }


    template<bool IsConst, Utils::Serializable ...Ts>
    template<class U>
    auto ListProxy<IsConst, Ts...>::ParseList(HeaderValue* val, std::string_view pattern) -> std::optional<std::vector<U>> {
        std::vector<U> res{};
        for (auto member : *val
                | std::views::split(',')
                | std::views::transform([](auto subrange) { return std::string_view{ subrange.begin(), subrange.end() }; })
                | std::views::transform([&](std::string_view input) { return Utils::Scan<U>(input, pattern); })) {
            if (!member)
                return std::nullopt;
            res.push_back(*member);
        }
        return res;
    }
}
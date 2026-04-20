#pragma once

namespace Thoth::Http::NHeaders {

    template<bool IsConst, Utils::Serializable ...Ts>
    ValueProxy<IsConst, Ts...>::ValueProxy(std::string_view key, HeaderType& headers, PatternType pattern)
            : key{ key }, headers{ headers }, inPattern{ pattern } { }


    template<bool IsConst, Utils::Serializable ...Ts>
    auto ValueProxy<IsConst, Ts...>::GetAsOpt() && -> std::optional<Type> {
        auto val{ headers.Get(key) };

        if (!val || (*val)->empty()) return std::nullopt;

        if constexpr (Single) {
            if (auto parsed{ Utils::Scan<Type>(**val, inPattern) }; parsed)
                return *parsed;
        } else {
            std::optional<Type> result;
            std::size_t i{};
            ([&] {
                if (auto parsed{ Utils::Scan<Ts>(**val, inPattern[i++]) }; parsed) {
                    result = *parsed;
                    return true;
                }
                return false;
            }() || ...);

            if (result) return result;
        }

        return std::nullopt;
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    auto ValueProxy<IsConst, Ts...>::Get() && -> std::expected<Type, HeaderErrorEnum> {
        auto val{ headers.Get(key) };

        if (!val)            return std::unexpected{ HeaderErrorEnum::NotFound };
        if ((*val)->empty()) return std::unexpected{ HeaderErrorEnum::EmptyValue };

        if constexpr (Single) {
            if (auto parsed{ Utils::Scan<Type>(**val, inPattern) }; parsed)
                return *parsed;
        } else {
            std::optional<Type> result;
            std::size_t i{};
            ([&] {
                if (auto parsed{ Utils::Scan<Ts>(**val, inPattern[i++]) }; parsed) {
                    result = *parsed;
                    return true;
                }
                return false;
            }() || ...);

            if (result) return *result;
        }

        return std::unexpected{ HeaderErrorEnum::InvalidFormat };
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    auto ValueProxy<IsConst, Ts...>::GetWithDefault(Type defaultValue) && -> std::expected<Type, InvalidHeaderFormat> {
        auto val{ headers.Get(key) };

        if (!val || (*val)->empty()) return defaultValue;

        if constexpr (Single) {
            if (auto parsed{ Utils::Scan<Type>(**val, inPattern) }; parsed)
                return *parsed;
        } else {
            std::optional<Type> result;
            std::size_t i{};
            ([&] {
                if (auto parsed{ Utils::Scan<Ts>(**val, inPattern[i++]) }; parsed) {
                    result = *parsed;
                    return true;
                }
                return false;
            }() || ...);

            if (result) return *result;
        }

        return std::unexpected{ InvalidHeaderFormat{} };
    }

    template<bool IsConst, Utils::Serializable ...Ts> // NOLINT(*-unconventional-assign-operator)
    template<class T>
        requires (!IsConst)
    void ValueProxy<IsConst, Ts...>::operator=(const T& newValue) && {
        std::move(*this).Set(newValue);
    }

    template<bool IsConst, Utils::Serializable ...Ts> // NOLINT(*-unconventional-assign-operator)
    template<class T>
        requires (!IsConst)
    void ValueProxy<IsConst, Ts...>::operator=(T&& newValue) && {
        std::move(*this).Set(std::move(newValue));
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    template<class T>
        requires (!IsConst)
    void ValueProxy<IsConst, Ts...>::Set(const T& newValue) && {
        headers.Set(key, std::format("{}", newValue));
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    template<class T>
        requires (!IsConst)
    void ValueProxy<IsConst, Ts...>::Set(T&& newValue) && {
        headers.Set(key, std::format("{}", std::move(newValue)));
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    template<class>
        requires (!IsConst)
    bool ValueProxy<IsConst, Ts...>::TrySet(std::string_view newValue) && {
        if constexpr (Single) {
            auto parsed{ Utils::Scan<Type>(newValue, inPattern) };
            if (!parsed) return false;
            std::move(*this).Set(*parsed);
            return true;
        } else {
            bool set{};
            std::size_t i{};
            ([&] {
                if (!set)
                    if (auto parsed{ Utils::Scan<Ts>(newValue, inPattern[i]) }; parsed) {
                        headers.Set(key, std::format("{}", *parsed));
                        set = true;
                    }
                ++i;
            }(), ...);
            return set;
        }
    }
}
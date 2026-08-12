#pragma once

namespace Thoth::Http::NHeaders {

    template<bool IsConst, Utils::Serializable ...Ts>
    ValueProxy<IsConst, Ts...>::ValueProxy(std::string_view key, HeaderType& headers, PatternType pattern)
            : m_key{ key }, m_headers{ headers }, m_inPattern{ pattern } { }


    template<bool IsConst, Utils::Serializable ...Ts>
    auto ValueProxy<IsConst, Ts...>::GetAsOpt() && -> std::optional<Type> {
        auto val{ m_headers.Get(m_key) };

        if (!val || (*val)->empty()) return std::nullopt;

        if constexpr (k_single) {
            if (auto parsed{ Utils::Scan<Type>(**val, m_inPattern) }; parsed)
                return *parsed;
        } else {
            std::optional<Type> result;
            std::size_t i{};
            (std::invoke([&] {
                if (auto parsed{ Utils::Scan<Ts>(**val, m_inPattern[i++]) }; parsed) {
                    result = *parsed;
                    return true;
                }
                return false;
            }) || ...);

            if (result) return result;
        }

        return std::nullopt;
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    auto ValueProxy<IsConst, Ts...>::Get() && -> std::expected<Type, HeaderErrorEnum> {
        auto val{ m_headers.Get(m_key) };

        if (!val)            return std::unexpected{ HeaderErrorEnum::NotFound };
        if ((*val)->empty()) return std::unexpected{ HeaderErrorEnum::EmptyValue };

        if constexpr (k_single) {
            if (auto parsed{ Utils::Scan<Type>(**val, m_inPattern) }; parsed)
                return *parsed;
        } else {
            std::optional<Type> result;
            std::size_t i{};
            (std::invoke([&] {
                if (auto parsed{ Utils::Scan<Ts>(**val, m_inPattern[i++]) }; parsed) {
                    result = *parsed;
                    return true;
                }
                return false;
            }) || ...);

            if (result) return *result;
        }

        return std::unexpected{ HeaderErrorEnum::InvalidFormat };
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    auto ValueProxy<IsConst, Ts...>::GetWithDefault(Type defaultValue) && -> std::expected<Type, InvalidHeaderFormat> {
        auto val{ m_headers.Get(m_key) };

        if (!val || (*val)->empty()) return defaultValue;

        if constexpr (k_single) {
            if (auto parsed{ Utils::Scan<Type>(**val, m_inPattern) }; parsed)
                return *parsed;
        } else {
            std::optional<Type> result;
            std::size_t i{};
            (std::invoke([&] {
                if (auto parsed{ Utils::Scan<Ts>(**val, m_inPattern[i++]) }; parsed) {
                    result = *parsed;
                    return true;
                }
                return false;
            }) || ...);

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
        m_headers.Set(m_key, std::format("{}", newValue));
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    template<class T>
        requires (!IsConst)
    void ValueProxy<IsConst, Ts...>::Set(T&& newValue) && {
        m_headers.Set(m_key, std::format("{}", std::move(newValue)));
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    template<class>
        requires (!IsConst)
    bool ValueProxy<IsConst, Ts...>::TrySet(std::string_view newValue) && {
        if constexpr (k_single) {
            auto parsed{ Utils::Scan<Type>(newValue, m_inPattern) };
            if (!parsed) return false;
            std::move(*this).Set(*parsed);
            return true;
        } else {
            bool set{};
            std::size_t i{};
            (std::invoke([&] {
                if (!set)
                    if (auto parsed{ Utils::Scan<Ts>(newValue, m_inPattern[i]) }; parsed) {
                        m_headers.Set(m_key, std::format("{}", *parsed));
                        set = true;
                    }
                ++i;
            }), ...);
            return set;
        }
    }
}
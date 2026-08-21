#pragma once
#include <Thoth/Utils/Functional.hpp>

namespace Thoth::Http::NHeaders {

    template<bool IsConst, Utils::Serializable ...Ts>
    MultiValueProxy<IsConst, Ts...>::MultiValueProxy(std::string_view key, HeaderType& headers, PatternType inPattern)
            : m_key{ key }, m_headers{ headers }, m_inPattern{ inPattern } { }


    template<bool IsConst, Utils::Serializable ...Ts>
    auto MultiValueProxy<IsConst, Ts...>::GetAsOpt() && -> std::optional<Type> {
        const auto values{ GetValues() };
        if (values.empty()) return std::nullopt;

        Type result;
        for (const auto val : values) {
            if (val->second.empty()) return std::nullopt;

            if constexpr (k_single) {
                if (auto parsed{ Utils::Scan<ElemType>(val->second, m_inPattern) }; parsed)
                    result.push_back(*parsed);
                else
                    return std::nullopt;
            } else {
                std::optional<ElemType> parsedValue;
                std::size_t i{};
                (std::invoke([&] {
                    if (auto parsed{ Utils::Scan<Ts>(val->second, m_inPattern[i++]) }; parsed) {
                        parsedValue = *parsed;
                        return true;
                    }
                    return false;
                }) || ...);

                if (!parsedValue) return std::nullopt;
                result.push_back(*parsedValue);
            }
        }

        return result;
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    auto MultiValueProxy<IsConst, Ts...>::Get() && -> std::expected<Type, HeaderErrorEnum> {
        const auto values{ GetValues() };
        if (values.empty()) return std::unexpected{ HeaderErrorEnum::NotFound };

        Type result;
        for (const auto val : values) {
            if (val->second.empty()) return std::unexpected{ HeaderErrorEnum::EmptyValue };

            if constexpr (k_single) {
                if (auto parsed{ Utils::Scan<ElemType>(val->second, m_inPattern) }; parsed)
                    result.push_back(*parsed);
                else
                    return std::unexpected{ HeaderErrorEnum::InvalidFormat };
            } else {
                std::optional<ElemType> parsedValue;
                std::size_t i{};
                (std::invoke([&] {
                    if (auto parsed{ Utils::Scan<Ts>(val->second, m_inPattern[i++]) }; parsed) {
                        parsedValue = *parsed;
                        return true;
                    }
                    return false;
                }) || ...);

                if (!parsedValue)
                    return std::unexpected{ HeaderErrorEnum::InvalidFormat };

                result.push_back(*parsedValue);
            }
        }

        return result;
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    auto MultiValueProxy<IsConst, Ts...>::GetWithDefault(Type defaultValue) &&
            -> std::expected<Type, InvalidHeaderFormat> {
        const auto values{ GetValues() };
        if (values.empty()) return defaultValue;

        Type result;
        for (const auto val : values) {
            if (val->second.empty()) return std::unexpected{ InvalidHeaderFormat{} };

            if constexpr (k_single) {
                if (auto parsed{ Utils::Scan<ElemType>(val->second, m_inPattern) }; parsed)
                    result.push_back(*parsed);
                else
                    return std::unexpected{ InvalidHeaderFormat{} };
            } else {
                std::optional<ElemType> parsedValue;
                std::size_t i{};
                (std::invoke([&] {
                    if (auto parsed{ Utils::Scan<Ts>(val->second, m_inPattern[i++]) }; parsed) {
                        parsedValue = *parsed;
                        return true;
                    }
                    return false;
                }) || ...);

                if (!parsedValue)
                    return std::unexpected{ InvalidHeaderFormat{} };

                result.push_back(*parsedValue);
            }
        }

        return result;
    }


    template<bool IsConst, Utils::Serializable ...Ts> // NOLINT(*-unconventional-assign-operator)
    template<std::ranges::range R>
    void MultiValueProxy<IsConst, Ts...>::operator=(R&& newValue) &&
        requires (!IsConst && std::convertible_to<std::ranges::range_reference_t<R>, ElemType>) {
        std::move(*this).Set(std::forward<R>(newValue));
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    template<std::ranges::range R>
    void MultiValueProxy<IsConst, Ts...>::Set(R&& newValue) &&
        requires (!IsConst && std::convertible_to<std::ranges::range_reference_t<R>, ElemType>) {
        while (m_headers.Remove(m_key)) {}

        for (auto&& value : newValue)
            m_headers.Add(m_key, std::format("{}", value));
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    template<class>
    void MultiValueProxy<IsConst, Ts...>::Add(const ElemType& newItem) &&
        requires (!IsConst) {
        if constexpr (k_single) {
            m_headers.Add(m_key, std::format("{}", newItem));
        } else {
            std::visit([&](const auto& item) {
                m_headers.Add(m_key, std::format("{}", item));
            }, newItem);
        }
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    template<class>
    bool MultiValueProxy<IsConst, Ts...>::TrySet(std::string_view newValue) &&
        requires (!IsConst) {
        HeaderValue temp{ newValue };
        std::optional<ElemType> parsedValue;

        if constexpr (k_single) {
            parsedValue = Utils::Scan<ElemType>(temp, m_inPattern);
        } else {
            std::size_t i{};
            (std::invoke([&] {
                if (!parsedValue)
                    parsedValue = Utils::Scan<Ts>(temp, m_inPattern[i]);
                ++i;
            }), ...);
        }

        if (!parsedValue) return false;

        std::move(*this).Set(std::vector<ElemType>{ *parsedValue });
        return true;
    }

    template<bool IsConst, Utils::Serializable ...Ts>
    auto MultiValueProxy<IsConst, Ts...>::GetValues() const -> ValuesType {
        return m_headers.GetAll(m_key);
    }
}

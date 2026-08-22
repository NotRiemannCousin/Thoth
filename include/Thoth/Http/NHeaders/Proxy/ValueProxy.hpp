#pragma once
#include <expected>
#include <Thoth/Http/NHeaders/_base.hpp>
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>

namespace Thoth::Http::NHeaders {

    //! @brief Typed proxy for a single header value.
    //!
    //! A `ValueProxy` adapts one header field to a typed value. The proxy itself is a short-lived accessor.
    //!
    //! @note The proxy is consumed by the `Get*` methods. Call `Get()`, `GetAsOpt()` or `GetWithDefault()` to
    //! materialize the parsed value.
    //!
    //! @tparam IsConst Whether the proxy can modify its `Headers` object.
    //! @tparam Ts Serializable value types accepted by the value proxy.
    template<bool IsConst, Utils::Serializable ...Ts>
    struct ValueProxy{
        static constexpr int k_count{ sizeof...(Ts) };
        static constexpr int k_single{ sizeof...(Ts) == 1 };


        static_assert(sizeof...(Ts) >= 1 && "At least 1 type must be provided.");

        using HeaderType  = std::conditional_t<IsConst, const Headers, Headers>;
        using PatternType = std::conditional_t<k_single, std::string_view, std::array<std::string_view, k_count>>;
#ifdef __cpp_pack_indexing
        using Type        = std::conditional_t<k_single, Ts...[0], std::variant<Ts...>>;
        // Lets go Microslop
#else
        template<class F, class...> struct First { using Type = F; };
        using Type        = std::conditional_t<k_single, typename First<Ts...>::Type, std::variant<Ts...>>;
#endif

        ValueProxy(ValueProxy&&) = delete;
        ValueProxy(const ValueProxy&) = delete;

        //! @brief Creates a proxy for `key` in `headers`.
        ValueProxy(std::string_view key, HeaderType& headers, PatternType inPattern = {});

        //! @brief Parses the header, returning no value when it is absent, empty or invalid.
        //! @note Consumes the proxy.
        std::optional<Type> GetAsOpt() &&;

        //! @brief Parses the header and reports absence, emptiness or invalid format.
        //! @note Consumes the proxy.
        std::expected<Type, HeaderErrorEnum> Get() &&;

        //! @brief Parses the header, using `defaultValue` when it is absent or empty.
        //! @return The parsed value, or `InvalidHeaderFormat` when a present value is invalid.
        //! @note Consumes the proxy.
        std::expected<Type, InvalidHeaderFormat> GetWithDefault(Type defaultValue) &&;

        //! @brief Replaces the header with `newValue`.
        template<class T>
            requires (!IsConst)
        void operator=(const T& newValue) &&; // NOLINT(*-unconventional-assign-operator)

        //! @brief Replaces the header with `newValue`.
        template<class T>
            requires (!IsConst)
        void operator=(T&& newValue) &&; // NOLINT(*-unconventional-assign-operator)

        //! @brief Replaces the header with `newValue`.
        template<class T>
            requires (!IsConst)
        void Set(const T& newValue) &&;

        //! @brief Replaces the header with `newValue`, moving it when possible.
        template<class T>
            requires (!IsConst)
        void Set(T&& newValue) &&;

        //! @brief Parses and replaces the header from a raw value.
        //! @return `true` when the value was parsed and stored. otherwise leaves the header unchanged.
        template<class = void>
            requires (!IsConst)
        bool TrySet(std::string_view newValue) &&;

    private:
        // const PatternType      m_outPattern;
        const PatternType      m_inPattern;
        const std::string_view m_key;
        HeaderType& m_headers;
    };
}

#include <Thoth/Http/NHeaders/Proxy/ValueProxy.tpp>
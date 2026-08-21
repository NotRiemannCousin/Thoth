#pragma once
#include <expected>
#include <Thoth/Http/NHeaders/_base.hpp>
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>

namespace Thoth::Http {
    struct Headers;
}

namespace Thoth::Http::NHeaders {

    //! @brief Typed proxy for multiple header values.
    //!
    //! A `MultiValueProxy` adapts one header field to a typed vector. The proxy itself is a short-lived accessor.
    //!
    //! @note Given that it can't be stored in any sense, it is intentionally not a `std::ranges::range`. Call `Get()`
    //! or `GetAsOpt()` to materialize the parsed `std::vector`.
    //!
    //! @tparam IsConst Whether the proxy can modify its `Headers` object.
    //! @tparam Ts Serializable value types accepted by the multi-value proxy.
    template<bool IsConst, Utils::Serializable ...Ts>
    struct MultiValueProxy {
        static constexpr std::size_t k_count{ sizeof...(Ts) };
        static constexpr bool k_single{ sizeof...(Ts) == 1 };

        static_assert(sizeof...(Ts) >= 1, "At least 1 type must be provided.");

        using HeaderType  = std::conditional_t<IsConst, const Headers, Headers>;
        using PatternType = std::conditional_t<k_single, std::string_view, std::array<std::string_view, k_count>>;
#ifdef __cpp_pack_indexing
        using ElemType    = std::conditional_t<k_single, Ts...[0], std::variant<Ts...>>;
        // Lets go Microslop, where is my pack indexing?
#else
        template<class F, class...> struct First { using Type = F; };
        using ElemType    = std::conditional_t<k_single, typename First<Ts...>::Type, std::variant<Ts...>>;
#endif

        using Type         = std::vector<ElemType>;
        using ValueType    = std::conditional_t<IsConst, const HeaderPair*, HeaderPair*>;
        using ValuesType   = std::vector<ValueType>;

        MultiValueProxy(MultiValueProxy&&) = delete;
        MultiValueProxy(const MultiValueProxy&) = delete;

        //! @brief Creates a proxy for `key` in `headers`.
        MultiValueProxy(std::string_view key, HeaderType& headers, PatternType inPattern = {});

        //! @brief Parses all values, returning no value when the header is absent or invalid.
        //! @note Consumes the proxy.
        std::optional<Type> GetAsOpt() &&;

        //! @brief Parses all values and reports absence, emptiness or invalid format.
        //! @note Consumes the proxy.
        std::expected<Type, HeaderErrorEnum> Get() &&;

        //! @brief Parses all values, using `defaultValue` when the header is absent or empty.
        //! @return The parsed values, or `InvalidHeaderFormat` when a present value is invalid.
        //! @note Consumes the proxy.
        std::expected<Type, InvalidHeaderFormat> GetWithDefault(Type defaultValue) &&;

        //! @brief Replaces the header with the serialized values from `newValue`.
        template<std::ranges::range R>
        void operator=(R&& newValue) &&
            requires (!IsConst && std::convertible_to<std::ranges::range_reference_t<R>, ElemType>);

        //! @brief Replaces the header with the serialized values from `newValue`.
        template<std::ranges::range R>
        void Set(R&& newValue) &&
            requires (!IsConst && std::convertible_to<std::ranges::range_reference_t<R>, ElemType>);

        //! @brief Adds one value to the header using the container's merge policy.
        template<class = void>
        void Add(const ElemType& newItem) &&
            requires (!IsConst);

        //! @brief Parses and replaces the header from a raw value.
        //! @return `true` when the value was parsed and stored; otherwise leaves the header unchanged.
        template<class = void>
        bool TrySet(std::string_view newValue) &&
            requires (!IsConst);

    private:
        [[nodiscard]] ValuesType GetValues() const;

        const PatternType m_inPattern;
        const std::string_view m_key;
        HeaderType& m_headers;
    };
}

#include <Thoth/Http/NHeaders/Proxy/MultiValueProxy.tpp>

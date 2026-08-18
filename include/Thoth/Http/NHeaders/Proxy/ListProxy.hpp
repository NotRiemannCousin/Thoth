#pragma once
#include <expected>
#include <Thoth/Http/NHeaders/_base.hpp>
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>

namespace Thoth::Http {
    struct Headers;
}

namespace Thoth::Http::NHeaders {

    //! @brief Typed proxy for a comma-separated header value.
    //!
    //! A `ListProxy` adapts one header field to a typed list. The proxy itself is a short-lived accessor.
    //!
    //! @note Given that it can't be stored in any sense, it is intentionally not a `std::ranges::range`. Call `Get()`
    //! or `GetAsOpt()` to materialize the parsed `std::vector`.
    //!
    //! @tparam IsConst Whether the proxy can modify its `Headers` object.
    //! @tparam Ts Serializable element types accepted by the list proxy.
    template<bool IsConst, Utils::Serializable ...Ts>
    struct ListProxy {
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

        using Type        = std::vector<ElemType>;

        ListProxy(ListProxy&&) = delete;
        ListProxy(const ListProxy&) = delete;

        //! @brief Creates a proxy for `key` in `headers`.
        ListProxy(std::string_view key, HeaderType& headers, PatternType inPattern = {});

        //! @brief Parses the header, returning no value when it is absent, empty or invalid.
        //! @note Consumes the proxy.
        std::optional<Type> GetAsOpt() &&;

        //! @brief Parses the header and reports absence, emptiness or invalid format.
        //! @note Consumes the proxy.
        std::expected<Type, HeaderErrorEnum> Get() &&;

        //! @brief Parses the header, using `defaultValue` when it is absent or empty.
        //! @return The parsed list, or `InvalidHeaderFormat` when a present value is invalid.
        //! @note Consumes the proxy.
        std::expected<Type, InvalidHeaderFormat> GetWithDefault(Type defaultValue) &&;

        //! @brief Replaces the header with the serialized values from `newValue`.
        template<std::ranges::range R>
            requires (!IsConst)
        void operator=(R&& newValue) &&; // NOLINT(*-unconventional-assign-operator)

        //! @brief Replaces the header with the serialized values from `newValue`.
        template<std::ranges::range R>
            requires (!IsConst)
        void Set(R&& newValue) &&;

        //! @brief Adds one item to the header using the container's merge policy.
        template<class = void>
            requires (!IsConst)
        void Add(const ElemType& newItem) &&;

        //! @brief Parses and replaces the header from a raw comma-separated value.
        //! @return `true` when the value was parsed and stored; otherwise leaves the header unchanged.
        template<class = void>
            requires (!IsConst)
        bool TrySet(std::string_view newValue) &&;

        //! @brief Parses a raw comma-separated header value into typed elements.
        //! @return The parsed list, or `std::nullopt` when any element is invalid.
        template<class U>
        static std::optional<std::vector<U>> ParseList(const HeaderValue* val, std::string_view pattern);

    private:
        const PatternType m_inPattern;
        const std::string_view m_key;
        HeaderType& m_headers;
    };
}

#include <Thoth/Http/NHeaders/Proxy/ListProxy.tpp>
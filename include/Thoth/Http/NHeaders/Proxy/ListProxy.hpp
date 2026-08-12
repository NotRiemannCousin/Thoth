#pragma once
#include <expected>
#include <Thoth/Http/NHeaders/_base.hpp>
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>

namespace Thoth::Http {
    struct Headers;
}

namespace Thoth::Http::NHeaders {

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

        ListProxy(std::string_view key, HeaderType& headers, PatternType inPattern = {});

        std::optional<Type> GetAsOpt() &&;
        std::expected<Type, HeaderErrorEnum> Get() &&;
        std::expected<Type, InvalidHeaderFormat> GetWithDefault(Type defaultValue) &&;

        template<std::ranges::range R>
            requires (!IsConst)
        void operator=(R&& newValue) &&; // NOLINT(*-unconventional-assign-operator)

        template<std::ranges::range R>
            requires (!IsConst)
        void Set(R&& newValue) &&;

        template<class = void>
            requires (!IsConst)
        void Add(const ElemType& newItem) &&;

        template<class = void>
            requires (!IsConst)
        bool TrySet(std::string_view newValue) &&;

        template<class U>
        static std::optional<std::vector<U>> ParseList(const HeaderValue* val, std::string_view pattern);

    private:
        const PatternType m_inPattern;
        const std::string_view m_key;
        HeaderType& m_headers;
    };
}

#include <Thoth/Http/NHeaders/Proxy/ListProxy.tpp>
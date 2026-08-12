#pragma once
#include <expected>
#include <Thoth/Http/NHeaders/_base.hpp>
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>

namespace Thoth::Http::NHeaders {

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

        ValueProxy(std::string_view key, HeaderType& headers, PatternType inPattern = {});

        std::optional<Type> GetAsOpt() &&;
        std::expected<Type, HeaderErrorEnum> Get() &&;
        std::expected<Type, InvalidHeaderFormat> GetWithDefault(Type defaultValue) &&;




        template<class T>
            requires (!IsConst)
        void operator=(const T& newValue) &&; // NOLINT(*-unconventional-assign-operator)
        template<class T>
            requires (!IsConst)
        void operator=(T&& newValue) &&; // NOLINT(*-unconventional-assign-operator)

        template<class T>
            requires (!IsConst)
        void Set(const T& newValue) &&;
        template<class T>
            requires (!IsConst)
        void Set(T&& newValue) &&;

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
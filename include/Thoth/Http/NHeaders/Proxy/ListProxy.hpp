#pragma once
#include <expected>
#include <Thoth/Http/NHeaders/_base.hpp>
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>


namespace Thoth::Http {
    struct Headers;
}

namespace Thoth::Http::NHeaders {

    template<Serializable T, bool IsConst>
    struct ListProxy{
        using Ts = std::vector<T>;
        using HeaderType = std::conditional_t<IsConst, const Headers, Headers>;

        ListProxy(ListProxy&&) = delete;
        ListProxy(const ListProxy&) = delete;

        ListProxy(std::string_view key, HeaderType& headers);

        std::optional<Ts> GetAsOpt() &&;
        std::expected<Ts, HeaderErrorEnum> Get() &&;
        std::expected<Ts, InvalidHeaderFormat> GetWithDefault(Ts defaultValue) &&;

        template<std::ranges::range R>
            requires (!IsConst)
        void operator=(R&& newValue) &&;

        template<std::ranges::range R>
            requires (!IsConst)
        void Set(R&& newValue) &&;


        template<class = void>
            requires (!IsConst)
        void Add(const T& newItem) &&;


        template<class = void>
            requires (!IsConst)
        bool TrySet(std::string_view newValue) &&;

        static std::optional<Ts> ParseList(HeaderValue* val);

        const std::string_view key;
        HeaderType& headers;
    };
}

#include <Thoth/Http/NHeaders/Proxy/ListProxy.tpp>
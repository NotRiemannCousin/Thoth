#pragma once
#include <expected>
#include <Thoth/Http/NHeaders/_base.hpp>
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>


namespace Thoth::Http {
    struct Headers;
}

namespace Thoth::Http::NHeaders {

    template<Serializable T>
    struct ListProxy{
        using Ts = std::vector<T>;

        ListProxy(ListProxy&&) = delete;
        ListProxy(const ListProxy&) = delete;

        ListProxy(std::string_view key, Headers& headers);

        std::optional<Ts> GetAsOpt() &&;
        std::expected<Ts, HeaderErrorEnum> Get() &&;
        std::expected<Ts, InvalidHeaderFormat> GetWithDefault(Ts defaultValue) &&;

        template<std::ranges::range R>
        void operator=(R&& newValue) &&;

        template<std::ranges::range R>
        void Set(R&& newValue) &&;

        void Add(const T& newItem) &&;

        bool TrySet(std::string_view newValue) &&;

        static std::optional<Ts> ParseList(HeaderValue* val);

        const std::string_view key;
        Headers &headers;
    };
}

#include <Thoth/Http/NHeaders/Proxy/ListProxy.tpp>
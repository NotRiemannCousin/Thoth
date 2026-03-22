#pragma once
#include <expected>
#include <Thoth/Http/NHeaders/_base.hpp>
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>

namespace Thoth::Http::NHeaders {

    template<Serializable T, bool IsConst>
    struct ValueProxy{
        using HeaderType = std::conditional_t<IsConst, const Headers, Headers>;

        ValueProxy(ValueProxy&&) = delete;
        ValueProxy(const ValueProxy&) = delete;

        ValueProxy(std::string_view key, HeaderType& headers);

        std::optional<T> GetAsOpt() &&;
        std::expected<T, HeaderErrorEnum> Get() &&;
        std::expected<T, InvalidHeaderFormat> GetWithDefault(T defaultValue) &&;

        template<class = void>
            requires (!IsConst)
        void operator=(const T& newValue) &&;
        template<class = void>
            requires (!IsConst)
        void operator=(T&& newValue) &&;

        template<class = void>
            requires (!IsConst)
        void Set(const T& newValue) &&;
        template<class = void>
            requires (!IsConst)
        void Set(T&& newValue) &&;

        template<class = void>
            requires (!IsConst)
        bool TrySet(std::string_view newValue) &&;

    private:
        const std::string_view key;
        HeaderType& headers;
    };
}

#include <Thoth/Http/NHeaders/Proxy/ValueProxy.tpp>
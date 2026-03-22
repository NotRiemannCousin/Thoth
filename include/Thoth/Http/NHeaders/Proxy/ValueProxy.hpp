#pragma once
#include <expected>
#include <Thoth/Http/NHeaders/_base.hpp>
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>

namespace Thoth::Http::NHeaders {

    template<Serializable T>
    struct ValueProxy{
        ValueProxy(ValueProxy&&) = delete;
        ValueProxy(const ValueProxy&) = delete;

        ValueProxy(std::string_view key, Headers& headers);

        std::optional<T> GetAsOpt() &&;
        std::expected<T, HeaderErrorEnum> Get() &&;
        std::expected<T, InvalidHeaderFormat> GetWithDefault(T defaultValue) &&;

        void operator=(const T& newValue) &&;
        void operator=(T&& newValue) &&;

        void Set(const T& newValue) &&;
        void Set(T&& newValue) &&;

        bool TrySet(std::string_view newValue) &&;

    private:
        const std::string_view key;
        Headers& headers;
    };
}

#include <Thoth/Http/NHeaders/Proxy/ValueProxy.tpp>
#pragma once

namespace Thoth::Http::NHeaders {
    template<Serializable T, bool IsConst>
    ValueProxy<T, IsConst>::ValueProxy(string_view key, HeaderType &headers) : key{ key }, headers{ headers } { }

    template<Serializable T, bool IsConst>
    std::optional<T> ValueProxy<T, IsConst>::GetAsOpt() && {
        return headers.Get(key).and_then(Scan<T>);
    }

    template<Serializable T, bool IsConst>
    std::expected<T, HeaderErrorEnum> ValueProxy<T, IsConst>::Get() && {
        auto val{ headers.Get(key) };

        if (!val) return std::unexpected{ HeaderErrorEnum::NotFound };
        if ((*val)->empty()) return std::unexpected{ HeaderErrorEnum::EmptyValue };

        auto parsed{ Scan<T>(*val) };
        if (!parsed) return std::unexpected{ HeaderErrorEnum::InvalidFormat };

        return *parsed;
    }

    template<Serializable T, bool IsConst>
    std::expected<T, InvalidHeaderFormat> ValueProxy<T, IsConst>::GetWithDefault(T defaultValue) && {
        auto val{ headers.Get(key) };

        if (!val || (*val)->empty()) return defaultValue;


        if (auto parsed{ Scan<T>(**val) }; !parsed)
            return *parsed;
        return std::unexpected{ InvalidHeaderFormat{} };
    }

    template<Serializable T, bool IsConst>
    template<class>
        requires (!IsConst)
    void ValueProxy<T, IsConst>::operator=(const T &newValue) && {
        Set(newValue);
    }

    template<Serializable T, bool IsConst>
    template<class>
        requires (!IsConst)
    void ValueProxy<T, IsConst>::operator=(T &&newValue) && {
        Set(std::move(newValue));
    }

    template<Serializable T, bool IsConst>
    template<class>
        requires (!IsConst)
    void ValueProxy<T, IsConst>::Set(const T &newValue) && {
        headers.Set(key, std::format("{}", newValue));
    }

    template<Serializable T, bool IsConst>
    template<class>
        requires (!IsConst)
    void ValueProxy<T, IsConst>::Set(T &&newValue) && {
        headers.Set(key, std::format("{}", newValue));
    }

    template<Serializable T, bool IsConst>
    template<class>
        requires (!IsConst)
    bool ValueProxy<T, IsConst>::TrySet(std::string_view newValue) && {
        auto val{ Scan<T>(newValue) };
        if (!val) return false;

        Set(val);
        return true;
    }
}



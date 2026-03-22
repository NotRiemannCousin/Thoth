#pragma once

namespace Thoth::Http::NHeaders {
    template<Serializable T>
    ValueProxy<T>::ValueProxy(string_view key, Headers &headers) : key{ key }, headers{ headers } { }

    template<Serializable T>
    std::optional<T> ValueProxy<T>::GetAsOpt() && {
        return headers.Get(key).and_then(Scan<T>);
    }

    template<Serializable T>
    std::expected<T, HeaderErrorEnum> ValueProxy<T>::Get() && {
        auto val{ headers.Get(key) };

        if (!val) return std::unexpected{ HeaderErrorEnum::NotFound };
        if ((*val)->empty()) return std::unexpected{ HeaderErrorEnum::EmptyValue };

        auto parsed{ Scan<T>(*val) };
        if (!parsed) return std::unexpected{ HeaderErrorEnum::InvalidFormat };

        return *parsed;
    }

    template<Serializable T>
    std::expected<T, InvalidHeaderFormat> ValueProxy<T>::GetWithDefault(T defaultValue) && {
        auto val{ headers.Get(key) };

        if (!val || (*val)->empty()) return defaultValue;


        if (auto parsed{ Scan<T>(**val) }; !parsed)
            return *parsed;
        return std::unexpected{ InvalidHeaderFormat{} };
    }

    template<Serializable T>
    void ValueProxy<T>::operator=(const T &newValue) && {
        Set(newValue);
    }

    template<Serializable T>
    void ValueProxy<T>::operator=(T &&newValue) && {
        Set(std::move(newValue));
    }

    template<Serializable T>
    void ValueProxy<T>::Set(const T &newValue) && {
        headers.Set(key, std::format("{}", newValue));
    }

    template<Serializable T>
    void ValueProxy<T>::Set(T &&newValue) && {
        headers.Set(key, std::format("{}", newValue));
    }

    template<Serializable T>
    bool ValueProxy<T>::TrySet(std::string_view newValue) && {
        auto val{ Scan<T>(newValue) };
        if (!val) return false;

        Set(val);
        return true;
    }
}



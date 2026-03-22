#pragma once
#include <format>
#include <optional>
#include <string_view>

#include <Thoth/String/Utils.hpp>

namespace Thoth::Http::NHeaders {
    template<class T, class ...Args>
    std::optional<T> Scan(std::string_view input, Args... args);
    // TODO: Implement Scan in a better way.


    template<class T>
    concept Serializable = requires(T t, std::string_view str) {
        { std::format("{}", t) } -> std::same_as<std::string>;
        { Scan<T>(str)         } -> std::same_as<std::optional<T>>;
    };


    enum class HeaderErrorEnum {
        NotFound,
        InvalidFormat,
        EmptyValue
    };
    struct InvalidHeaderFormat {};





    template<class T>
        requires std::integral<T>
    std::optional<T> Scan(std::string_view input, int base = 10) {
        if (base <= 0 || base > 36) return std::nullopt;

        T value;
        String::Trim(input);
        auto [_, ec] = std::from_chars(input.data(), input.data() + input.size(), value, base);

        if (ec != std::errc()) return std::nullopt;
        return value;
    }

    template<>
    inline std::optional<string> Scan<string>(std::string_view input) {
        return String::TrimmedStr(input);
    }
}

#pragma once
#include <Thoth/NJson/ErrorDefinitions.hpp>
#include <Thoth/Http/ErrorDefinitions.hpp>

namespace Thoth {
    struct GenericError {
        std::string error{};
    };

    using ThothErrorBase = std::variant<
        NJson::JsonParseError,
        NJson::JsonGetError,
        NJson::JsonFindError,
        NJson::JsonSearchError,
        NJson::JsonWrongTypeError,
        Http::UrlParseErrorEnum,
        Http::ConnectionErrorEnum,
        Http::MessageParseErrorEnum,
        GenericError
    >;

    struct ThothError : ThothErrorBase {
        using ThothErrorBase::variant;
        using ThothErrorBase::operator=;


        constexpr static auto FromError();

        template <class T>
        constexpr bool Is() const;

        template <class T>
        constexpr T As() const;

        template<class T>
        std::optional<T> Ensure() const;


        template <class T>
        constexpr bool operator==(const T& rhs) const;

        constexpr bool operator==(const ThothError&) const = default;
    };

    template<class T>
    using ThothResult = std::expected<T, ThothError>;
    using ThothResultOper = ThothResult<std::monostate>;

    using ThothUnex = std::unexpected<ThothError>;
}

#include <Thoth/ThothError.tpp>
#pragma once
#include <Hermes/_base/ConnectionErrorEnum.hpp>

#include <Thoth/NJson/Definitions.hpp>

#include <vector>


namespace Thoth::Http {
    struct JsonParseError {
        size_t idx;
        char c;
    };

    struct JsonGetError {
        NJson::Key key;
    };
    struct JsonFindError {
        NJson::Key key;
        std::vector<NJson::Key> currentPath;
    };
    struct JsonSearchError {
    };
    struct JsonWrongTypeError {
        template<class T>
        static constexpr size_t IndexOf{};

        size_t idxExpected{};
        size_t idxGot{};
    };

    // TODO: FUTURE: high obscure, change to some fancy way in the future, but that will do
    // std::variant<Null, String, Number, Bool, Object, Array>;
    template<> constexpr size_t JsonWrongTypeError::IndexOf<NJson::Null  >{ 0 };
    template<> constexpr size_t JsonWrongTypeError::IndexOf<NJson::String>{ 1 };
    template<> constexpr size_t JsonWrongTypeError::IndexOf<NJson::Number>{ 2 };
    template<> constexpr size_t JsonWrongTypeError::IndexOf<NJson::Bool  >{ 3 };
    template<> constexpr size_t JsonWrongTypeError::IndexOf<NJson::Object>{ 4 };
    template<> constexpr size_t JsonWrongTypeError::IndexOf<NJson::Array >{ 5 };

    enum class UrlParseErrorEnum {
        EmptyUrl,
        InvalidScheme,
        IllFormed,
        HostIsRequired,
        InvalidPort
    };

    using ConnectionErrorEnum = Hermes::ConnectionErrorEnum;

    enum class MessageParseErrorEnum {
        InvalidStartLine,
        InvalidVersion,
        InvalidHeaders,
        VersionNeedsContentLength
    };

    struct GenericError {
        std::string error{};
    };

    struct ExchangeError : std::variant<
        JsonParseError,
        JsonGetError,
        JsonFindError,
        JsonSearchError,
        JsonWrongTypeError,
        UrlParseErrorEnum,
        ConnectionErrorEnum,
        MessageParseErrorEnum,
        GenericError
    > { // first time I'm using inheritance in the project lol
    };

    template<class T>
    using ExchangeResult = std::expected<T, ExchangeError>;
}

#include <Thoth/Http/ExchangeError.tpp>
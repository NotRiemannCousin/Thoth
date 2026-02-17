#pragma once
#include <cstdint>
#include <vector>
#include <Hermes/_base/ConnectionErrorEnum.hpp>

#include <Thoth/NJson/Json.hpp>


namespace Thoth::Http {
    struct JsonParseError {
        size_t idx;
        char c;
    };

    struct JsonSearchError {
        NJson::Key key;
        std::vector<NJson::Key> currentPath;
    };

    enum class UrlParseErrorEnum {
        EmptyUrl,
        InvalidScheme,
        IllFormed,
        InvalidPort
    };

    using ConnectionErrorEnum = Hermes::ConnectionErrorEnum;

    enum class RequestBuildErrorEnum {
        InvalidResponse,
        InvalidVersion,
        InvalidHeaders,
        VersionNeedsContentLength
    };


    struct RequestError : std::variant<
        JsonParseError,
        JsonSearchError,
        UrlParseErrorEnum,
        ConnectionErrorEnum,
        RequestBuildErrorEnum
    > { // first time I'm using inheritance in the project lol
        using JsonParseError = JsonParseError;
        using JsonSearchError = JsonSearchError;
        using UrlParseError = UrlParseErrorEnum;
        using ConnectionError = ConnectionErrorEnum;
        using RequestBuildError = RequestBuildErrorEnum;
    };
}

#include <Thoth/Http/RequestError.tpp>
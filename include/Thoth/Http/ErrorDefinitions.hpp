#pragma once

namespace Hermes {
    enum class ConnectionErrorEnum;
}

namespace Thoth::Http {
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
}

#include <Thoth/Http/ErrorDefinitions.tpp>
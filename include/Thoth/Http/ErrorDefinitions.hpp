#pragma once

namespace Hermes {
    enum class ConnectionErrorEnum;
}

namespace Thoth::Http {
    //! @see Thoth::ThothError "ThothError" is the centralized error type of this library.
    enum class UrlParseErrorEnum {
        EmptyUrl,
        InvalidScheme,
        IllFormed,
        HostIsRequired,
        InvalidPort
    };

    //! @see Thoth::ThothError "ThothError" is the centralized error type of this library.
    using ConnectionErrorEnum = Hermes::ConnectionErrorEnum;

    //! @see Thoth::ThothError "ThothError" is the centralized error type of this library.
    enum class MessageParseErrorEnum {
        InvalidStartLine,
        InvalidVersion,
        InvalidHeaders,
        HeadersTooLarge,
        VersionNeedsContentLength
    };
}

#include <Thoth/Http/ErrorDefinitions.tpp>
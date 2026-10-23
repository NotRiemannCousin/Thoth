#pragma once
#include <expected>
#include <cstddef>
#include <format>
#include <string_view>
#include <utility>

namespace Thoth::Http {
    enum class StatusCodeEnum : std::uint16_t {
        // Informational responses
        Continue                      = 100,
        SwitchingProtocols            = 101,
        ProcessingDeprecated          = 102,
        EarlyHints                    = 103,


        // Successful responses
        Ok                            = 200,
        Created                       = 201,
        Accepted                      = 202,
        NonAuthoritativeInformation   = 203,
        NoContent                     = 204,
        ResetContent                  = 205,
        PartialContent                = 206,
        MultiStatus                   = 207, // (WebDAV)
        AlreadyReported               = 208, // (WebDAV)
        ImUsed                        = 226, // (HTTP Delta encoding)
        
        // Redirection messages
        MultipleChoices               = 300,
        MovedPermanently              = 301,
        Found                         = 302,
        SeeOther                      = 303,
        NotModified                   = 304,
        UseProxyDeprecated            = 305,
        Unused                        = 306,
        TemporaryRedirect             = 307,
        PermanentRedirect             = 308,


        // Client error responses
        BadRequest                    = 400,
        Unauthorized                  = 401,
        PaymentRequired               = 402,
        Forbidden                     = 403,
        NotFound                      = 404,
        MethodNotAllowed              = 405,
        NotAcceptable                 = 406,
        ProxyAuthenticationRequired   = 407,
        RequestTimeout                = 408,
        Conflict                      = 409,
        Gone                          = 410,
        LengthRequired                = 411,
        PreconditionFailed            = 412,
        ContentTooLarge               = 413,
        UriTooLong                    = 414,
        UnsupportedMediaType          = 415,
        RangeNotSatisfiable           = 416,
        ExpectationFailed             = 417,
        ImATeapot                     = 418,
        MisdirectedRequest            = 421,
        UnprocessableContent          = 422, // (WebDAV)
        Locked                        = 423, // (WebDAV)
        FailedDependency              = 424, // (WebDAV)
        TooEarlyExperimental          = 425,
        UpgradeRequired               = 426,
        PreconditionRequired          = 428,
        TooManyRequests               = 429,
        RequestHeaderFieldsTooLarge   = 431,
        UnavailableForLegalReasons    = 451,


        // Server error responses
        InternalServerError           = 500,
        NotImplemented                = 501,
        BadGateway                    = 502,
        ServiceUnavailable            = 503,
        GatewayTimeout                = 504,
        HttpVersionNotSupported       = 505,
        VariantAlsoNegotiates         = 506,
        InsufficientStorage           = 507, // (WebDAV)
        LoopDetected                  = 508, // (WebDAV)
        NotExtended                   = 510,
        NetworkAuthenticationRequired = 511,
    };


    enum class StatusTypeEnum {
        Informational,
        Successful,
        Redirection,
        ClientError,
        ServerError
    };

    //! @brief Check what type of code StatusCodeEnum is, the first digit of it's number.
    //!
    //! 1XX => Informational
    //! 2XX => Successful
    //! 3XX => Redirection
    //! 4XX => ClientError
    //! 5XX => ServerError
    constexpr StatusTypeEnum GetStatusType(StatusCodeEnum code);



    template<class T>
    using WebResult = std::expected<T, StatusCodeEnum>;


    using WebResultOper = WebResult<void>;
}

#include <Thoth/Http/Response/StatusCodeEnum.tpp>
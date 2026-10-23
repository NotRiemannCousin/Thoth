#pragma once

namespace Thoth::Http {

    constexpr StatusTypeEnum GetStatusType(const StatusCodeEnum code) {
        return static_cast<StatusTypeEnum>(static_cast<size_t>(code) / 100 - 1);
    }

    inline std::string_view StatusText(const StatusCodeEnum code) noexcept {
        switch (code) {
            case StatusCodeEnum::Continue                    : return "Continue";
            case StatusCodeEnum::SwitchingProtocols          : return "Switching Protocols";
            case StatusCodeEnum::ProcessingDeprecated        : return "Processing";
            case StatusCodeEnum::EarlyHints                  : return "Early Hints";
            case StatusCodeEnum::Ok                          : return "OK";
            case StatusCodeEnum::Created                     : return "Created";
            case StatusCodeEnum::Accepted                    : return "Accepted";
            case StatusCodeEnum::NonAuthoritativeInformation : return "Non-Authoritative Information";
            case StatusCodeEnum::NoContent                   : return "No Content";
            case StatusCodeEnum::ResetContent                : return "Reset Content";
            case StatusCodeEnum::PartialContent              : return "Partial Content";
            case StatusCodeEnum::MultiStatus                 : return "Multi-Status";
            case StatusCodeEnum::AlreadyReported             : return "Already Reported";
            case StatusCodeEnum::ImUsed                      : return "IM Used";
            case StatusCodeEnum::MultipleChoices             : return "Multiple Choices";
            case StatusCodeEnum::MovedPermanently            : return "Moved Permanently";
            case StatusCodeEnum::Found                       : return "Found";
            case StatusCodeEnum::SeeOther                    : return "See Other";
            case StatusCodeEnum::NotModified                 : return "Not Modified";
            case StatusCodeEnum::UseProxyDeprecated          : return "Use Proxy";
            case StatusCodeEnum::Unused                      : return "(Unused)";
            case StatusCodeEnum::TemporaryRedirect           : return "Temporary Redirect";
            case StatusCodeEnum::PermanentRedirect           : return "Permanent Redirect";
            case StatusCodeEnum::BadRequest                  : return "Bad Request";
            case StatusCodeEnum::Unauthorized                : return "Unauthorized";
            case StatusCodeEnum::PaymentRequired             : return "Payment Required";
            case StatusCodeEnum::Forbidden                   : return "Forbidden";
            case StatusCodeEnum::NotFound                    : return "Not Found";
            case StatusCodeEnum::MethodNotAllowed            : return "Method Not Allowed";
            case StatusCodeEnum::NotAcceptable               : return "Not Acceptable";
            case StatusCodeEnum::ProxyAuthenticationRequired : return "Proxy Authentication Required";
            case StatusCodeEnum::RequestTimeout              : return "Request Timeout";
            case StatusCodeEnum::Conflict                    : return "Conflict";
            case StatusCodeEnum::Gone                        : return "Gone";
            case StatusCodeEnum::LengthRequired              : return "Length Required";
            case StatusCodeEnum::PreconditionFailed          : return "Precondition Failed";
            case StatusCodeEnum::ContentTooLarge             : return "Content Too Large";
            case StatusCodeEnum::UriTooLong                  : return "URI Too Long";
            case StatusCodeEnum::UnsupportedMediaType        : return "Unsupported Media Type";
            case StatusCodeEnum::RangeNotSatisfiable         : return "Range Not Satisfiable";
            case StatusCodeEnum::ExpectationFailed           : return "Expectation Failed";
            case StatusCodeEnum::ImATeapot                   : return "I'm a teapot";
            case StatusCodeEnum::MisdirectedRequest          : return "Misdirected Request";
            case StatusCodeEnum::UnprocessableContent        : return "Unprocessable Content";
            case StatusCodeEnum::Locked                      : return "Locked";
            case StatusCodeEnum::FailedDependency            : return "Failed Dependency";
            case StatusCodeEnum::TooEarlyExperimental        : return "Too Early";
            case StatusCodeEnum::UpgradeRequired             : return "Upgrade Required";
            case StatusCodeEnum::PreconditionRequired        : return "Precondition Required";
            case StatusCodeEnum::TooManyRequests             : return "Too Many Requests";
            case StatusCodeEnum::RequestHeaderFieldsTooLarge : return "Request Header Fields Too Large";
            case StatusCodeEnum::UnavailableForLegalReasons  : return "Unavailable For Legal Reasons";
            case StatusCodeEnum::InternalServerError         : return "Internal Server Error";
            case StatusCodeEnum::NotImplemented              : return "Not Implemented";
            case StatusCodeEnum::BadGateway                  : return "Bad Gateway";
            case StatusCodeEnum::ServiceUnavailable          : return "Service Unavailable";
            case StatusCodeEnum::GatewayTimeout              : return "Gateway Timeout";
            case StatusCodeEnum::HttpVersionNotSupported     : return "HTTP Version Not Supported";
            case StatusCodeEnum::VariantAlsoNegotiates       : return "Variant Also Negotiates";
            case StatusCodeEnum::InsufficientStorage         : return "Insufficient Storage";
            case StatusCodeEnum::LoopDetected                : return "Loop Detected";
            case StatusCodeEnum::NotExtended                 : return "Not Extended";
            case StatusCodeEnum::NetworkAuthenticationRequired: return "Network Authentication Required";
        }
        return "Unknown";
    }
}

template<>
struct std::formatter<Thoth::Http::StatusCodeEnum> {
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    auto format(const Thoth::Http::StatusCodeEnum s, auto& ctx) const {
        return std::format_to(ctx.out(), "{} {}",
            std::to_underlying(s), Thoth::Http::StatusText(s));
    }
};
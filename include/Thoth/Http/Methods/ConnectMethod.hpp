#pragma once
#include <Thoth/Http/Methods/_base/MethodConcept.hpp>

namespace Thoth::Http {
    struct ConnectMethod {
        static constexpr std::string_view MethodName() { return "CONNECT"; }
        static constexpr bool IsSafe()       { return false; }
        static constexpr bool IsIdempotent() { return false; }

        static WebResultOper ValidateRequest(std::string_view body, const Url&, const Headers&) {
            if (!body.empty())
                return std::unexpected{ StatusCodeEnum::BadRequest };
            return {};
        }

        static WebResultOper ValidateResponse(StatusCodeEnum statusCode, std::string_view body, const Url&, const Headers&) {
            if (GetStatusType(statusCode) == StatusTypeEnum::SUCCESSFUL)
                return {};

            if ((statusCode == StatusCodeEnum::NoContent || statusCode == StatusCodeEnum::NotModified) && !body.empty())
                return std::unexpected{ StatusCodeEnum::BadGateway };

            return {};
        }
    };

    static_assert(MethodConcept<ConnectMethod>);
}

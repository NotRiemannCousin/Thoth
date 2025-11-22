#pragma once
#include <Thoth/Http/Methods/_base/MethodConcept.hpp>

namespace Thoth::Http {
    struct ConnectMethod {
        static constexpr std::string_view MethodName() { return "CONNECT"; }
        static constexpr bool IsSafe()       { return false; }
        static constexpr bool IsIdempotent() { return false; }

        static WebResultOper ValidateRequest(string_view body, const Url&, const Headers&) {
            if (!body.empty())
                return std::unexpected{ StatusCodeEnum::BAD_REQUEST };
            return {};
        }

        static WebResultOper ValidateResponse(StatusCodeEnum statusCode, string_view body, const Url&, const Headers&) {
            if (GetStatusType(statusCode) == StatusTypeEnum::SUCCESSFUL)
                return {};

            if ((statusCode == StatusCodeEnum::NO_CONTENT || statusCode == StatusCodeEnum::NOT_MODIFIED) && !body.empty())
                return std::unexpected{ StatusCodeEnum::BAD_GATEWAY };

            return {};
        }
    };

    static_assert(MethodConcept<ConnectMethod>);
}

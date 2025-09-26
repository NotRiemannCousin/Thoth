#pragma once
#include <Thoth/Http/HttpMethods/_base/HttpMethodConcept.hpp>

namespace Thoth::Http {
    struct HttpConnectMethod {
        static constexpr std::string_view MethodName() { return "CONNECT"; }
        static constexpr bool IsSafe()       { return false; }
        static constexpr bool IsIdempotent() { return false; }

        static WebResultOper ValidateRequest(string_view body, const HttpUrl& url, const HttpHeaders& headers) {
            if (!body.empty())
                return std::unexpected{ HttpStatusCodeEnum::BAD_REQUEST };
            return {};
        }

        static WebResultOper ValidateResponse(HttpStatusCodeEnum statusCode, string_view body, const HttpUrl& url, const HttpHeaders& headers) {
            if (GetStatusType(statusCode) == HttpStatusTypeEnum::SUCCESSFUL)
                return {};

            if ((statusCode == HttpStatusCodeEnum::NO_CONTENT || statusCode == HttpStatusCodeEnum::NOT_MODIFIED) && !body.empty())
                return std::unexpected{ HttpStatusCodeEnum::BAD_GATEWAY };

            return {};
        }
    };

    static_assert(HttpMethodConcept<HttpConnectMethod>);
}

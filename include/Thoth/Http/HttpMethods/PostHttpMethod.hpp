#pragma once
#include <Thoth/Http/HttpMethods/_base/HttpMethodConcept.hpp>

namespace Thoth::Http {
    struct HttpPostMethod {
        static constexpr std::string_view MethodName() { return "POST"; }
        static constexpr bool IsSafe()       { return false; }
        static constexpr bool IsIdempotent() { return false; }

        static WebResultOper ValidateRequest(string_view body, const HttpUrl& url, const HttpHeaders& headers) {
            return {};
        }

        static WebResultOper ValidateResponse(HttpStatusCodeEnum statusCode, string_view body, const HttpUrl& url, const HttpHeaders& headers) {
            if ((statusCode == HttpStatusCodeEnum::NO_CONTENT || statusCode == HttpStatusCodeEnum::NOT_MODIFIED) && !body.empty())
                return std::unexpected{ HttpStatusCodeEnum::BAD_GATEWAY };


            if (statusCode == HttpStatusCodeEnum::CREATED && !headers.Exists("Location"))
                return std::unexpected{ HttpStatusCodeEnum::BAD_GATEWAY };

            return {};
        }
    };

    static_assert(HttpMethodConcept<HttpPostMethod>);
}

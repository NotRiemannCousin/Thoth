#pragma once
#include <Thoth/Http/HttpMethods/_base/HttpMethodConcept.hpp>

namespace Thoth::Http {
    struct HttpTraceMethod {
        static constexpr std::string_view MethodName() { return "TRACE"; }
        static constexpr bool IsSafe()       { return true; }
        static constexpr bool IsIdempotent() { return true; }

        static WebResultOper ValidateRequest(string_view body, const HttpUrl&, const HttpHeaders&) {
            if (!body.empty())
                return std::unexpected{ HttpStatusCodeEnum::BAD_REQUEST };
            return {};
        }

        static WebResultOper ValidateResponse(HttpStatusCodeEnum statusCode, string_view body, const HttpUrl&, const HttpHeaders& headers) {
            string str;
            auto ref = std::ref(str);

            if (statusCode == HttpStatusCodeEnum::OK)
                if (headers.Get("Content-Type").value_or(ref).get() != "message/http")
                    return std::unexpected{ HttpStatusCodeEnum::BAD_GATEWAY };


            if ((statusCode == HttpStatusCodeEnum::NO_CONTENT || statusCode == HttpStatusCodeEnum::NOT_MODIFIED) && !body.empty())
                return std::unexpected{ HttpStatusCodeEnum::BAD_GATEWAY };

            return {};
        }
    };

    static_assert(HttpMethodConcept<HttpTraceMethod>);
}

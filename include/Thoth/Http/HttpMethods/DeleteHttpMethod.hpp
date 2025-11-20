#pragma once
#include <Thoth/Http/HttpMethods/_base/HttpMethodConcept.hpp>

namespace Thoth::Http {
    struct HttpDeleteMethod {
        static constexpr std::string_view MethodName() { return "DELETE"; }
        static constexpr bool IsSafe()       { return false; }
        static constexpr bool IsIdempotent() { return true; }

        static WebResultOper ValidateRequest(string_view body, const HttpUrl&, const HttpHeaders&) {
            if (!body.empty())
                return std::unexpected{ HttpStatusCodeEnum::BAD_REQUEST };
            return {};
        }

        static WebResultOper ValidateResponse(HttpStatusCodeEnum statusCode, string_view body, const HttpUrl&, const HttpHeaders&) {
            if ((statusCode == HttpStatusCodeEnum::NO_CONTENT || statusCode == HttpStatusCodeEnum::NOT_MODIFIED) && !body.empty())
                return std::unexpected{ HttpStatusCodeEnum::BAD_GATEWAY };

            return {};
        }
    };

    static_assert(HttpMethodConcept<HttpDeleteMethod>);
}

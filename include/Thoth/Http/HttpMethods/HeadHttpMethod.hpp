#pragma once
#include <Thoth/Http/HttpMethods/_base/HttpMethodConcept.hpp>

namespace Thoth::Http {
    struct HttpHeadMethod {
        static constexpr std::string_view MethodName() { return "HEAD"; }
        static constexpr bool IsSafe()       { return true; }
        static constexpr bool IsIdempotent() { return true; }

        static WebResultOper ValidateRequest(string_view body, const HttpUrl&, const HttpHeaders&) {
            if (!body.empty())
                return std::unexpected{ HttpStatusCodeEnum::BAD_REQUEST };
            return {};
        }

        static WebResultOper ValidateResponse(HttpStatusCodeEnum, string_view body, const HttpUrl&, const HttpHeaders&) {
            if (!body.empty())
                return std::unexpected{ HttpStatusCodeEnum::BAD_GATEWAY };
            return {};
        }
    };

    static_assert(HttpMethodConcept<HttpHeadMethod>);
}

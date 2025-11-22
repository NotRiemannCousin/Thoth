#pragma once
#include <Thoth/Http/Methods/_base/MethodConcept.hpp>

namespace Thoth::Http {
    struct PostMethod {
        static constexpr std::string_view MethodName() { return "POST"; }
        static constexpr bool IsSafe()       { return false; }
        static constexpr bool IsIdempotent() { return false; }

        static WebResultOper ValidateRequest(string_view body, const Url& url, const Headers& headers) {
            return {};
        }

        static WebResultOper ValidateResponse(StatusCodeEnum statusCode, string_view body, const Url& url, const Headers& headers) {
            if ((statusCode == StatusCodeEnum::NO_CONTENT || statusCode == StatusCodeEnum::NOT_MODIFIED) && !body.empty())
                return std::unexpected{ StatusCodeEnum::BAD_GATEWAY };


            if (statusCode == StatusCodeEnum::CREATED && !headers.Exists("Location"))
                return std::unexpected{ StatusCodeEnum::BAD_GATEWAY };

            return {};
        }
    };

    static_assert(MethodConcept<PostMethod>);
}

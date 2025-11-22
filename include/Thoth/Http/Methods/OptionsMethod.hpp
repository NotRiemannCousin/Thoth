#pragma once
#include <Thoth/Http/Methods/_base/MethodConcept.hpp>

namespace Thoth::Http {
    struct OptionsMethod {
        static constexpr std::string_view MethodName() { return "OPTIONS"; }
        static constexpr bool IsSafe()       { return true; }
        static constexpr bool IsIdempotent() { return true; }

        static WebResultOper ValidateRequest(string_view body, const Url& url, const Headers& headers) {
            if (!body.empty())
                return std::unexpected{ StatusCodeEnum::BAD_REQUEST };
            return {};
        }

        static WebResultOper ValidateResponse(StatusCodeEnum statusCode, string_view body, const Url& url, const Headers& headers) {
            if ((statusCode == StatusCodeEnum::NO_CONTENT || statusCode == StatusCodeEnum::NOT_MODIFIED) && !body.empty())
                return std::unexpected{ StatusCodeEnum::BAD_GATEWAY };


            if (statusCode == StatusCodeEnum::OK && !headers.Exists("Allow"))
                return std::unexpected{ StatusCodeEnum::BAD_GATEWAY };

            return {};
        }
    };

    static_assert(MethodConcept<OptionsMethod>);
}

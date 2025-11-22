#pragma once
#include <Thoth/Http/Methods/_base/MethodConcept.hpp>

namespace Thoth::Http {
    struct HeadMethod {
        static constexpr std::string_view MethodName() { return "HEAD"; }
        static constexpr bool IsSafe()       { return true; }
        static constexpr bool IsIdempotent() { return true; }

        static WebResultOper ValidateRequest(string_view body, const Url&, const Headers&) {
            if (!body.empty())
                return std::unexpected{ StatusCodeEnum::BAD_REQUEST };
            return {};
        }

        static WebResultOper ValidateResponse(StatusCodeEnum, string_view body, const Url&, const Headers&) {
            if (!body.empty())
                return std::unexpected{ StatusCodeEnum::BAD_GATEWAY };
            return {};
        }
    };

    static_assert(MethodConcept<HeadMethod>);
}

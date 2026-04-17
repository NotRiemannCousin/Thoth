#pragma once
#include <Thoth/Http/Methods/_base/MethodConcept.hpp>

namespace Thoth::Http {
    struct DeleteMethod {
        static constexpr std::string_view MethodName() { return "DELETE"; }
        static constexpr bool IsSafe()       { return false; }
        static constexpr bool IsIdempotent() { return true; }

        static WebResultOper ValidateRequest(std::string_view body, const Url&, const Headers&) {
            if (!body.empty())
                return std::unexpected{ StatusCodeEnum::BadRequest };
            return {};
        }

        static WebResultOper ValidateResponse(StatusCodeEnum statusCode, std::string_view body, const Url&, const Headers&) {
            if ((statusCode == StatusCodeEnum::NoContent || statusCode == StatusCodeEnum::NotModified) && !body.empty())
                return std::unexpected{ StatusCodeEnum::BadGateway };

            return {};
        }
    };

    static_assert(MethodConcept<DeleteMethod>);
}

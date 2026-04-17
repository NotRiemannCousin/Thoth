#pragma once
#include <Thoth/Http/Methods/_base/MethodConcept.hpp>

namespace Thoth::Http {
    struct PutMethod {
        static constexpr std::string_view MethodName() { return "PUT"; }
        static constexpr bool IsSafe()       { return false; }
        static constexpr bool IsIdempotent() { return true; }

        static WebResultOper ValidateRequest(std::string_view, const Url&, const Headers&) {
            return {};
        }

        static WebResultOper ValidateResponse(StatusCodeEnum statusCode, std::string_view body, const Url&, const Headers& headers) {
            if ((statusCode == StatusCodeEnum::NoContent || statusCode == StatusCodeEnum::NotModified) && !body.empty())
                return std::unexpected{ StatusCodeEnum::BadGateway };

            if (statusCode == StatusCodeEnum::Created && !headers.Exists("Location"))
                return std::unexpected{ StatusCodeEnum::BadGateway };

            return {};
        }
    };

    static_assert(MethodConcept<PutMethod>);
}

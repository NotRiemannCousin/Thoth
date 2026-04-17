#pragma once
#include <Thoth/Http/Methods/_base/MethodConcept.hpp>

namespace Thoth::Http {
    struct TraceMethod {
        static constexpr std::string_view MethodName() { return "TRACE"; }
        static constexpr bool IsSafe()       { return true; }
        static constexpr bool IsIdempotent() { return true; }

        static WebResultOper ValidateRequest(std::string_view body, const Url&, const Headers&) {
            if (!body.empty())
                return std::unexpected{ StatusCodeEnum::BadRequest };
            return {};
        }

        static WebResultOper ValidateResponse(StatusCodeEnum statusCode, std::string_view body, const Url&, const Headers& headers) {
            std::string str;
            auto ref{ &str };

            if (statusCode == StatusCodeEnum::Ok)
                if (*headers.Get("Content-Type").value_or(ref) != "message/http")
                    return std::unexpected{ StatusCodeEnum::BadGateway };


            if ((statusCode == StatusCodeEnum::NoContent || statusCode == StatusCodeEnum::NotModified) && !body.empty())
                return std::unexpected{ StatusCodeEnum::BadGateway };

            return {};
        }
    };

    static_assert(MethodConcept<TraceMethod>);
}

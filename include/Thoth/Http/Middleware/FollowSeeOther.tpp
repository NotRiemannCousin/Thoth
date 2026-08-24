#pragma once
#include <Thoth/Http/Middleware/FollowSeeOther.hpp>

namespace Thoth::Http {
    template<class Next>
    auto FollowSeeOther(Next next, uint32_t maxHops) {
        return [next = std::move(next), maxHops]<MethodConcept Method, BodyConcept Body>
            (Request<Method, Body> request) mutable -> Client::ExpResponse<SeeOtherMethod<Method>, Body>
            requires HandlerConcept<Next, Method, Body>
                  && HandlerConcept<Next, SeeOtherMethod<Method>, Body>
                  && std::default_initializable<Body>
        {
            using ResultMethod = SeeOtherMethod<Method>;

            auto first{ next(request) };
            if (!first) return std::unexpected{ first.error() };

            if (first->status != StatusCodeEnum::SeeOther) {
                if constexpr (std::same_as<Method, ResultMethod>)
                    return first;
                else
                    return ThothUnex{ GenericError{ "Expected 303 See Other to establish " + std::string{ ResultMethod::MethodName() } } };
            }

            auto location{ first->headers.Location().GetAsOpt() };
            if (!location) return ThothUnex{ GenericError{ "303 See Other without Location" } };
            auto url{ request.url.Resolve(*location) };
            if (!url) return std::unexpected{ url.error() };

            Request<ResultMethod, Body> current{ RequestHead{
                .url     = std::move(*url),
                .version = request.version,
                .headers = request.headers,
            } };
            current.headers.Remove("content-type");
            current.headers.Remove("content-length");


            for (uint32_t hop{ 1 }; ; ++hop) {
                auto response{ next(current) };
                if (!response) return response;
                if (response->status != StatusCodeEnum::SeeOther) return response;
                if (hop >= maxHops)
                    return ThothUnex{ GenericError{ "Too many redirects" } };

                auto nextLocation{ response->headers.Location().GetAsOpt() };
                if (!nextLocation) return response;
                auto nextUrl{ current.url.Resolve(*nextLocation) };
                if (!nextUrl) return response;
                current.url = std::move(*nextUrl);
            }
        };
    }
}
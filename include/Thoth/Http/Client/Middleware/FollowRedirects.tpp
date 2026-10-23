#pragma once
#include <Thoth/String/Utils.hpp>

namespace Thoth::Http::details_ {
    inline bool SameOrigin(const Url& a, const Url& b) {
        const auto schemeA{ a.GetScheme() }, schemeB{ b.GetScheme() };
        if (!std::ranges::equal(schemeA, schemeB, &String::CaseInsensitiveCompare)) return false;

        const auto authA{ a.GetAuthority() }, authB{ b.GetAuthority() };
        if (!authA || !authB) return false;
        if (authA->GetHostString() != authB->GetHostString()) return false;

        const auto portA{ authA->port.or_else([&] { return GetDefaultPort(schemeA); }) };
        const auto portB{ authB->port.or_else([&] { return GetDefaultPort(schemeB); }) };
        return portA == portB;
    }
}

namespace Thoth::Http {
    template<class Next>
    auto FollowRedirects(Next next, uint32_t maxHops) {
        return [next = std::move(next), maxHops]<MethodConcept Method, BodyConcept Body>
            (Request<Method, Body> request) mutable -> Client::ExpResponse<Method, Body>
            requires HandlerConcept<Next, Method, Body> && std::copyable<Body>
        {
            using enum StatusCodeEnum;
            Request<Method, Body> current{ std::move(request) };

            for (uint32_t hop{}; ; ++hop) {
                auto response{ next(current) };
                if (!response) return response;

                const bool isRedirect{ response->status == TemporaryRedirect || response->status == PermanentRedirect };
                if (!isRedirect) return response;
                if (hop + 1 >= maxHops)
                    return ThothUnex{ GenericError{ "Too many redirects" } };

                auto location{ response->headers.Location().GetAsOpt() };
                if (!location) return response;

                auto newUrl{ current.url.Resolve(*location) };
                if (!newUrl) return response;

                if (!details_::SameOrigin(current.url, *newUrl)) {
                    current.headers.Remove("authorization");
                    current.headers.Remove("cookie");
                }
                current.url = std::move(*newUrl);
                if (const auto authority{ current.url.GetAuthority() })
                    current.headers.Host().Set(authority->GetHostString());
            }
        };
    }
}
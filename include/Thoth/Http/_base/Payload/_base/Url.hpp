#pragma once
#include <string>
#include <utility>

#include <Thoth/Http/_base/Payload/_base.hpp>
#include <Thoth/Http/Url/Url.hpp>

namespace Thoth::Http {
    template<BodyConcept TargetBody>
        requires std::constructible_from<TargetBody, std::string>
    struct IntoPayload<TargetBody, Url> {
        Url val;

        IntoPayload(Url url);

        operator IntoRequestPayload<TargetBody>() &&;
        operator IntoResponsePayload<TargetBody>() &&;
    };
}

#include <Thoth/Http/_base/Payload/_base/Url.tpp>

namespace Thoth::Http {
    static_assert(IntoRequestPayloadConcept<std::string, Url>);
    static_assert(IntoResponsePayloadConcept<std::string, Url>);
}
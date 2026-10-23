#pragma once

#include <Thoth/Http/_base/Payload/_base.hpp>

namespace Thoth::Http {
    template<BodyConcept TargetBody>
        requires std::ranges::output_range<TargetBody, char>
    struct IntoPayload<TargetBody, ThothError> {
        ThothError err;

        IntoPayload(ThothError e);

        operator IntoResponsePayload<TargetBody>() &&;
    };
}

#include <Thoth/Http/_base/Payload/_base/ThothError.tpp>

namespace Thoth::Http {
    static_assert(IntoResponsePayloadConcept<std::string, ThothError>);
}
#pragma once

namespace Thoth::Http {
    template<BodyConcept TargetBody>
        requires std::default_initializable<TargetBody>
    IntoPayload<TargetBody, std::monostate>::IntoPayload(std::monostate) {}

    template<BodyConcept TargetBody>
        requires std::default_initializable<TargetBody>
    IntoPayload<TargetBody, std::monostate>::operator IntoRequestPayload<TargetBody>() && {
        return {};
    }

    template<BodyConcept TargetBody>
        requires std::default_initializable<TargetBody>
    IntoPayload<TargetBody, std::monostate>::operator IntoResponsePayload<TargetBody>() && {
        return { { .status = StatusCodeEnum::NoContent }, {} };
    }
}

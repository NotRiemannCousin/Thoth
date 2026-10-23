#pragma once
#include <string>
#include <utility>

#include <Thoth/Http/_base/Payload/_base.hpp>

namespace Thoth::Http {
    template<BodyConcept TargetBody>
        requires std::default_initializable<TargetBody>
    struct IntoPayload<TargetBody, StatusCodeEnum> {
        StatusCodeEnum val;

        IntoPayload(StatusCodeEnum e);

        operator IntoResponsePayload<TargetBody>() &&;
    };
    
    template<BodyConcept TargetBody, class T>
        requires IntoResponsePayloadConcept<TargetBody, T>
    struct IntoPayload<TargetBody, std::pair<StatusCodeEnum, T>> {
        std::pair<StatusCodeEnum, T> val;

        IntoPayload(std::pair<StatusCodeEnum, T> e);
        IntoPayload(StatusCodeEnum status, T body);

        operator IntoResponsePayload<TargetBody>() &&;
    };
}

#include <Thoth/Http/_base/Payload/_base/StatusCodeEnum.tpp>

namespace Thoth::Http {
    static_assert(IntoResponsePayloadConcept<std::string, StatusCodeEnum>);
}
#pragma once
#include <optional>

#include <Thoth/Http/_base/Payload/_base.hpp>

namespace Thoth::Http {
    template<BodyConcept TargetBody, class T>
        requires (IntoRequestPayloadConcept<TargetBody, T> || IntoResponsePayloadConcept<TargetBody, T>) &&
            std::default_initializable<TargetBody>
    struct IntoPayload<TargetBody, std::optional<T>> {
        std::optional<T> val;
        StatusCodeEnum status;

        IntoPayload(std::optional<T> e, StatusCodeEnum status = StatusCodeEnum::NotFound);

        template<class = void>
            requires IntoRequestPayloadConcept<TargetBody, T>
        operator IntoRequestPayload<TargetBody>() &&;
        template<class = void>
            requires IntoResponsePayloadConcept<TargetBody, T>
        operator IntoResponsePayload<TargetBody>() &&;
    };
}

#include <Thoth/Http/_base/Payload/_base/Optional.tpp>
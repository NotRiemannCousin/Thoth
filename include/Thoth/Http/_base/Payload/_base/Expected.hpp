#pragma once

#include <expected>
#include <Thoth/Http/_base/Payload/_base.hpp>

#include <Thoth/Utils/Traits.hpp>

namespace Thoth::Http {

    template<BodyConcept TargetBody, class T, class E>
        requires (IntoRequestPayloadConcept<TargetBody, Utils::NormalizedExpectedValue<T>> && IntoRequestPayloadConcept<TargetBody, E>) ||
                 (IntoResponsePayloadConcept<TargetBody, Utils::NormalizedExpectedValue<T>> && IntoResponsePayloadConcept<TargetBody, E>)
    struct IntoPayload<TargetBody, std::expected<T, E>> {
        std::expected<T, E> val;

        IntoPayload(std::expected<T, E> e);

        template<class = void>
            requires (IntoRequestPayloadConcept<TargetBody, Utils::NormalizedExpectedValue<T>> && IntoRequestPayloadConcept<TargetBody, E>)
        operator IntoRequestPayload<TargetBody>() &&;

        template<class = void>
            requires (IntoResponsePayloadConcept<TargetBody, Utils::NormalizedExpectedValue<T>> && IntoResponsePayloadConcept<TargetBody, E>)
        operator IntoResponsePayload<TargetBody>() &&;
    };
}

#include <Thoth/Http/_base/Payload/_base/Expected.tpp>
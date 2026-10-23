#pragma once

#include <variant>

#include <Thoth/Http/_base/Payload/_base.hpp>

namespace Thoth::Http {
    template<BodyConcept TargetBody, class... Ts>
        requires (IntoRequestPayloadConcept<TargetBody, Ts> && ...) || (IntoResponsePayloadConcept<TargetBody, Ts> && ...)
    struct IntoPayload<TargetBody, std::variant<Ts...>> {
        std::variant<Ts...> val;

        IntoPayload(std::variant<Ts...> e);

        template<class = void>
            requires (IntoRequestPayloadConcept<TargetBody, Ts> && ...)
        operator IntoRequestPayload<TargetBody>() &&;
        template<class = void>
            requires (IntoResponsePayloadConcept<TargetBody, Ts> && ...)
        operator IntoResponsePayload<TargetBody>() &&;
    };
}

#include <Thoth/Http/_base/Payload/_base/Variant.tpp>
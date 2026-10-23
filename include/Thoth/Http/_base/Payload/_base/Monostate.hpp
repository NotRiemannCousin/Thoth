#pragma once

#include <variant>

#include <Thoth/Http/_base/Payload/_base.hpp>

namespace Thoth::Http {
    template<BodyConcept TargetBody>
        requires std::default_initializable<TargetBody>
    struct IntoPayload<TargetBody, std::monostate> {
        IntoPayload(std::monostate);

        operator IntoRequestPayload<TargetBody>() &&;
        operator IntoResponsePayload<TargetBody>() &&;
    };
}

#include <Thoth/Http/_base/Payload/_base/Monostate.tpp>

namespace Thoth::Http {
    static_assert(IntoRequestPayloadConcept<std::string, std::monostate>);
    static_assert(IntoResponsePayloadConcept<std::string, std::monostate>);
}
#pragma once

namespace Thoth::Http {
    template<BodyConcept TargetBody, class T, class E>
        requires (IntoRequestPayloadConcept<TargetBody, Utils::NormalizedExpectedValue<T>> && IntoRequestPayloadConcept<TargetBody, E>) ||
                 (IntoResponsePayloadConcept<TargetBody, Utils::NormalizedExpectedValue<T>> && IntoResponsePayloadConcept<TargetBody, E>)
    IntoPayload<TargetBody, std::expected<T, E>>::IntoPayload(std::expected<T, E> e) : val{ std::move(e) } {}

    template<BodyConcept TargetBody, class T, class E>
        requires (IntoRequestPayloadConcept<TargetBody, Utils::NormalizedExpectedValue<T>> && IntoRequestPayloadConcept<TargetBody, E>) ||
                 (IntoResponsePayloadConcept<TargetBody, Utils::NormalizedExpectedValue<T>> && IntoResponsePayloadConcept<TargetBody, E>)
    template<class>
        requires (IntoRequestPayloadConcept<TargetBody, Utils::NormalizedExpectedValue<T>> && IntoRequestPayloadConcept<TargetBody, E>)
    IntoPayload<TargetBody, std::expected<T, E>>::operator IntoRequestPayload<TargetBody>() && {
        if (val) {
            if constexpr (std::is_void_v<T>) return IntoPayload<TargetBody, std::monostate>{ std::monostate{} };
            else                             return IntoPayload<TargetBody, T>{ std::move(*val) };
        }
        return IntoPayload<TargetBody, E>{ std::move(val.error()) };
    }

    template<BodyConcept TargetBody, class T, class E>
        requires (IntoRequestPayloadConcept<TargetBody, Utils::NormalizedExpectedValue<T>> && IntoRequestPayloadConcept<TargetBody, E>) ||
                 (IntoResponsePayloadConcept<TargetBody, Utils::NormalizedExpectedValue<T>> && IntoResponsePayloadConcept<TargetBody, E>)
    template<class>
        requires (IntoResponsePayloadConcept<TargetBody, Utils::NormalizedExpectedValue<T>> && IntoResponsePayloadConcept<TargetBody, E>)
    IntoPayload<TargetBody, std::expected<T, E>>::operator IntoResponsePayload<TargetBody>() && {
        if (val) {
            if constexpr (std::is_void_v<T>) return IntoPayload<TargetBody, std::monostate>{ std::monostate{} };
            else                             return IntoPayload<TargetBody, T>{ std::move(*val) };
        }
        return IntoPayload<TargetBody, E>{ std::move(val.error()) };
    }
}
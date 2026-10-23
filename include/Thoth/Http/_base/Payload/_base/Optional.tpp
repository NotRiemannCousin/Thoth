#pragma once

namespace Thoth::Http {
    template<BodyConcept TargetBody, class T>
        requires (IntoRequestPayloadConcept<TargetBody, T> || IntoResponsePayloadConcept<TargetBody, T>) &&
                std::default_initializable<TargetBody>
    IntoPayload<TargetBody, std::optional<T>>::IntoPayload(std::optional<T> e, StatusCodeEnum status)
        : val { std::move(e) }, status { status } {}

    template<BodyConcept TargetBody, class T>
        requires (IntoRequestPayloadConcept<TargetBody, T> || IntoResponsePayloadConcept<TargetBody, T>) &&
                std::default_initializable<TargetBody>
    template<class>
        requires IntoRequestPayloadConcept<TargetBody, T>
    IntoPayload<TargetBody, std::optional<T>>::operator IntoRequestPayload<TargetBody>() && {
        if (val) return IntoPayload<TargetBody, T>{ std::move(val.value()) };
        return {};
    }

    template<BodyConcept TargetBody, class T>
        requires (IntoRequestPayloadConcept<TargetBody, T> || IntoResponsePayloadConcept<TargetBody, T>) &&
                std::default_initializable<TargetBody>
    template<class>
        requires IntoResponsePayloadConcept<TargetBody, T>
    IntoPayload<TargetBody, std::optional<T>>::operator IntoResponsePayload<TargetBody>() && {
        if (val) return IntoPayload<TargetBody, T>{ std::move(val.value()) };
        return { { .status = status }, {} };
    }
}
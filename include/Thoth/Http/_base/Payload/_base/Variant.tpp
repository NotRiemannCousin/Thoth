#pragma once

namespace Thoth::Http {
    template<BodyConcept TargetBody, class... Ts>
        requires (IntoRequestPayloadConcept<TargetBody, Ts> && ...) || (IntoResponsePayloadConcept<TargetBody, Ts> && ...)
    IntoPayload<TargetBody, std::variant<Ts...>>::IntoPayload(std::variant<Ts...> e) : val { std::move(e) } {}

    template<BodyConcept TargetBody, class... Ts>
        requires (IntoRequestPayloadConcept<TargetBody, Ts> && ...) || (IntoResponsePayloadConcept<TargetBody, Ts> && ...)
    template<class>
        requires (IntoRequestPayloadConcept<TargetBody, Ts> && ...)
    IntoPayload<TargetBody, std::variant<Ts...>>::operator IntoRequestPayload<TargetBody>() && {
        return std::visit([&]<class T>(T&& e) {
            return static_cast<IntoRequestPayload<TargetBody>>(IntoPayloadFor<TargetBody, T>{ std::move(e) });
        }, std::move(val));
    }

    template<BodyConcept TargetBody, class... Ts>
        requires (IntoRequestPayloadConcept<TargetBody, Ts> && ...) || (IntoResponsePayloadConcept<TargetBody, Ts> && ...)
    template<class>
        requires (IntoResponsePayloadConcept<TargetBody, Ts> && ...)
    IntoPayload<TargetBody, std::variant<Ts...>>::operator IntoResponsePayload<TargetBody>() && {
        return std::visit([&]<class T>(T&& e) {
            return static_cast<IntoResponsePayload<TargetBody>>(IntoPayloadFor<TargetBody, T>{ std::move(e) });
        }, std::move(val));
    }
}
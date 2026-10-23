#pragma once

namespace Thoth::Http {
    template<BodyConcept TargetBody>
        requires std::default_initializable<TargetBody>
    IntoPayload<TargetBody, StatusCodeEnum>::IntoPayload(StatusCodeEnum e) : val{ e } {}

    template<BodyConcept TargetBody>
        requires std::default_initializable<TargetBody>
    IntoPayload<TargetBody, StatusCodeEnum>::operator IntoResponsePayload<TargetBody>() && {
        return { { .status = val }, {} };
    }


    template<BodyConcept TargetBody, class T>
        requires IntoResponsePayloadConcept<TargetBody, T>
    IntoPayload<TargetBody, std::pair<StatusCodeEnum, T>>::IntoPayload(std::pair<StatusCodeEnum, T> e) : val { std::move(e) } {}

    template<BodyConcept TargetBody, class T>
        requires IntoResponsePayloadConcept<TargetBody, T>
    IntoPayload<TargetBody, std::pair<StatusCodeEnum, T>>::IntoPayload(StatusCodeEnum status, T body) : val { { status, std::move(body) } } {}

    template<BodyConcept TargetBody, class T>
        requires IntoResponsePayloadConcept<TargetBody, T>
    IntoPayload<TargetBody, std::pair<StatusCodeEnum, T>>::operator IntoResponsePayload<TargetBody>() && {
        IntoResponsePayload<TargetBody> response{ IntoPayload<TargetBody, T>{ std::move(val.second) } };
        response.first.status = val.first;
        return std::move(response);
    }
}
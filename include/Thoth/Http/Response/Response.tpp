#pragma once

namespace Thoth::Http {

    template<MethodConcept Method, BodyConcept Body>
    template<class>
        requires std::same_as<Body, std::string>
    std::expected<NJson::Json, ThothError> Response<Method, Body>::AsJson() const {
        return NJson::Json::Parse(body);
    }

    template<MethodConcept Method, BodyConcept Body>
    bool Response<Method, Body>::Successful() const {
        return GetStatusType(status) == StatusTypeEnum::Successful;
    }

    template<MethodConcept Method, BodyConcept Body>
    std::expected<Response<Method, Body>, ThothError> Response<Method, Body>::EnsureSuccess(Response &&response) {
        if (response.Successful())
            return std::move(response);

        return std::unexpected{
            ThothError{ GenericError{
                std::format("Invalid response status code: {}", std::to_underlying(response.status))
            } }
        };
    }

    template<MethodConcept Method, BodyConcept Body>
    Body Response<Method, Body>::MoveBody() && {
        return std::move(body);
    }

    template<BodyConcept TargetBody, MethodConcept Method>
    struct IntoPayload<TargetBody, Response<Method, TargetBody>> {
        Response<Method, TargetBody> val;

        IntoPayload(Response<Method, TargetBody> r) : val{ std::move(r) } {}

        operator IntoResponsePayload<TargetBody>() && {
            return IntoResponsePayload<TargetBody>{
                std::move(static_cast<ResponseHead&&>(val)),
                std::move(val.body)
            };
        }
    };
}

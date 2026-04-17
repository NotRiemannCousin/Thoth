#pragma once

namespace Thoth::Http {

    template<ResponseBodyConcept Body>
        requires requires(Body b){ { b.push_back({}) }; }
    auto GetInserterIterator(Body& body) {
        return std::back_inserter(body);
    }

    template<ResponseBodyConcept Body>
    auto GetInserterIterator(Body& body) {
        return std::ranges::begin(body);
    }



    template<MethodConcept Method, ResponseBodyConcept Body>
    Response<Method, Body>::Response(VersionEnum version, StatusCodeEnum status, std::string&& statusMessage,
            Headers&& headers, Body&& body) : version{ version },  status{ status },
            statusMessage{ std::move(statusMessage) },  headers{ std::move(headers) },
            body{ std::move(body) } { }

    template<MethodConcept Method, ResponseBodyConcept Body>
    Response<Method, Body>::Response(ResponseHead &&head, Body &&body) : version{ head.version },  status{ head.status },
            statusMessage{ std::move(head.statusMessage) },  headers{ std::move(head.headers) },
            body{ std::move(body) } { }

    template<MethodConcept Method, ResponseBodyConcept Body>
    template<class>
        requires std::same_as<Body, std::string>
    std::expected<NJson::Json, RequestError> Response<Method, Body>::AsJson() const {
        return NJson::Json::Parse(body);
    }

    template<MethodConcept Method, ResponseBodyConcept Body>
    bool Response<Method, Body>::Successful() const {
        return GetStatusType(status) == StatusTypeEnum::SUCCESSFUL;
    }

    template<MethodConcept Method, ResponseBodyConcept Body>
    std::expected<Response<Method, Body>, RequestError> Response<Method, Body>::EnsureSuccess(Response &&response) {
        if (response.Successful())
            return std::move(response);

        return std::unexpected{
            RequestError{ GenericError{
                std::format("Invalid response status code: {}", std::to_underlying(response.status))
            } }
        };
    }

    template<MethodConcept Method, ResponseBodyConcept Body>
    Body Response<Method, Body>::MoveBody() && {
        return std::move(body);
    }
}

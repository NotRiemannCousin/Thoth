#pragma once

namespace Thoth::Http {

    template<WritableBodyConcept Body>
        requires requires(Body b){ { b.push_back({}) }; }
    auto GetInserterIterator(Body& body) {
        return std::back_inserter(body);
    }

    template<WritableBodyConcept Body>
    auto GetInserterIterator(Body& body) {
        return std::ranges::begin(body);
    }

    template<MethodConcept Method, WritableBodyConcept Body>
    template<class>
        requires std::same_as<Body, std::string>
    std::expected<NJson::Json, ExchangeError> Response<Method, Body>::AsJson() const {
        return NJson::Json::Parse(body);
    }

    template<MethodConcept Method, WritableBodyConcept Body>
    bool Response<Method, Body>::Successful() const {
        return GetStatusType(status) == StatusTypeEnum::SUCCESSFUL;
    }

    template<MethodConcept Method, WritableBodyConcept Body>
    std::expected<Response<Method, Body>, ExchangeError> Response<Method, Body>::EnsureSuccess(Response &&response) {
        if (response.Successful())
            return std::move(response);

        return std::unexpected{
            ExchangeError{ GenericError{
                std::format("Invalid response status code: {}", std::to_underlying(response.status))
            } }
        };
    }

    template<MethodConcept Method, WritableBodyConcept Body>
    Body Response<Method, Body>::MoveBody() && {
        return std::move(body);
    }


    namespace details_ {
        template<class Stream>
        using ResponseParseStage = ParseStage<Stream, ResponseHead>;

        template<class Stream, WritableBodyConcept Body>
        using ResponseParseCompleteStage = ParseCompleteStage<Stream, ResponseHead, Body>;
    }
}

#pragma once

namespace Thoth::Http {

    template<MethodConcept Method, BodyConcept Body>
    ResponseEntry<Method, Body>::ResponseEntry(ResponseType response)
        : m_response{ std::move(response) } {}

    template<MethodConcept Method, BodyConcept Body>
    template<class Source>
        requires IntoResponsePayloadConcept<Body, Source>
    ResponseEntry<Method, Body>::ResponseEntry(Source&& source) {
        using Payload = IntoResponsePayload<Body>;
        auto [head, body]{ static_cast<Payload>(IntoPayloadFor<Body, Source>{ std::forward<Source>(source) }) };
        m_response = ResponseType{ std::move(head), std::move(body) };
    }

    template<MethodConcept Method, BodyConcept Body>
    ResponseEntry<Method, Body>::operator ResponseType() && {
        return std::move(m_response);
    }
}

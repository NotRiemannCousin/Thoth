#pragma once

namespace Thoth::Http {

    template<MethodConcept Method, BodyConcept Body>
    RequestEntry<Method, Body>::RequestEntry(RequestType request)
        : m_request{ std::move(request) } {}

    template<MethodConcept Method, BodyConcept Body>
    template<class Source>
        requires IntoRequestPayloadConcept<Body, Source>
    RequestEntry<Method, Body>::RequestEntry(Source&& source) {
        using Payload = IntoRequestPayload<Body>;
        auto [head, body]{ static_cast<Payload>(IntoPayloadFor<Body, Source>{ std::forward<Source>(source) }) };
        m_request = RequestType{ std::move(head), std::move(body) };
    }

    template<MethodConcept Method, BodyConcept Body>
    RequestEntry<Method, Body>::operator RequestType() && {
        return std::move(m_request);
    }
}

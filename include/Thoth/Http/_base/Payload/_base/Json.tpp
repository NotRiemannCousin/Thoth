#pragma once
#include <algorithm>

namespace Thoth::Http::details_ {
    template<BodyConcept TargetBody, class T>
    JsonIntoPayload<TargetBody, T>::JsonIntoPayload(T body) : val{ std::move(body) } {}

    template<BodyConcept TargetBody, class T>
    JsonIntoPayload<TargetBody, T>::operator IntoResponsePayload<TargetBody>() && {
        ResponseHead head{};
        head.headers.ContentType() = NHeaders::MimeTypes::appJson;
        TargetBody body;
        std::format_to(std::back_inserter(body), "{}", val);
        return { std::move(head), std::move(body) };
    }

    template<BodyConcept TargetBody, class T>
    JsonIntoPayload<TargetBody, T>::operator IntoRequestPayload<TargetBody>() && {
        RequestHead head{};
        head.headers.ContentType() = NHeaders::MimeTypes::appJson;

        TargetBody body;
        std::format_to(std::back_inserter(body), "{}", val);
        return { std::move(head), std::move(body) };
    }
}
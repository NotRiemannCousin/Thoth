#pragma once
#include <algorithm>
#include <string>

namespace Thoth::Http::details_ {
    template<class T>
    TextIntoPayload<T>::TextIntoPayload(T body) : body{ std::move(body) } {}

    template<class T>
    TextIntoPayload<T>::operator IntoResponsePayload<std::string>() && {
        ResponseHead head{};
        head.headers.ContentType() = NHeaders::MimeTypes::textPlain;
        return { std::move(head), std::move(body) };
    }

    template<class T>
    TextIntoPayload<T>::operator IntoRequestPayload<std::string>() && {
        RequestHead head{};
        head.headers.ContentType() = NHeaders::MimeTypes::textPlain;
        return { std::move(head), std::move(body) };
    }
}
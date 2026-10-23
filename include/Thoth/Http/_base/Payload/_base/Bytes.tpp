#pragma once
#include <algorithm>

namespace Thoth::Http::details_ {
    template<class R>
    BytesIntoPayload<R>::BytesIntoPayload(R&& body) : body{ std::from_range, std::forward<R>(body) } {}

    template<class R>
    BytesIntoPayload<R>::operator IntoResponsePayload<std::vector<std::byte>>() && {
        ResponseHead head{};
        head.headers.ContentType() = NHeaders::MimeTypes::appOctetStream;
        return { std::move(head), std::move(body) };
    }

    template<class R>
    BytesIntoPayload<R>::operator IntoRequestPayload<std::vector<std::byte>>() && {
        RequestHead head{};
        head.headers.ContentType() = NHeaders::MimeTypes::appOctetStream;
        return { std::move(head), std::move(body) };
    }
}
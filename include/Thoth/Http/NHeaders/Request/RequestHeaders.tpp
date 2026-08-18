#pragma once

namespace Thoth::Http {
    template<std::ranges::input_range R>
    WebResult<RequestHeaders> RequestHeaders::Parse(R&& headers, size_t maxHeadersLength) {
        return Headers::Parse(std::forward<R>(headers), maxHeadersLength)
                .transform([](Headers&& header){ return RequestHeaders{ std::move(header) }; });
    }
}
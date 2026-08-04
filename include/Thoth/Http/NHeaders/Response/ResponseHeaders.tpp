#pragma once

namespace Thoth::Http {
    template<std::ranges::input_range R>
    WebResult<ResponseHeaders> ResponseHeaders::Parse(R& headers, size_t maxHeadersLength) {
        return Headers::Parse(headers, maxHeadersLength)
                .transform([](Headers&& header){ return ResponseHeaders{ std::move(header) }; });
    }
}
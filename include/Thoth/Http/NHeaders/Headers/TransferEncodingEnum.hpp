#pragma once

namespace Thoth::Http::NHeaders {
    //! @brief Transfer codings used to delimit or transform an HTTP message body (RFC 9112 §§6.1, 7).
    //!
    //! These values are used by `Transfer-Encoding`. `Chunked` is the standard HTTP/1.1 framing coding, the other
    //! values name registered transfer codings.
    enum class TransferEncodingEnum {
        //! Chunked transfer coding.
        Chunked,
        //! Unix compress transfer coding.
        Compress,
        //! DEFLATE transfer coding.
        Deflate,
        //! GNU zip transfer coding.
        Gzip
    };
}

#include <Thoth/Http/NHeaders/Headers/TransferEncodingEnum.tpp>

static_assert(Thoth::Utils::Serializable<Thoth::Http::NHeaders::TransferEncodingEnum>);
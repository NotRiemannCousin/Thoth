#pragma once

namespace Thoth::Http::NHeaders {
    //! @brief Content codings applied to an HTTP representation (RFC 9110 §8.4).
    //!
    //! These values are used by `Content-Encoding` to describe the codings that have been applied to the
    //! representation.
    enum class ContentEncodingEnum {
        //! GNU zip coding (`gzip`).
        Gzip,
        //! Unix compress coding (`compress`).
        Compress,
        //! DEFLATE coding (`deflate`).
        Deflate,
        //! Brotli coding (`br`).
        Br,
        //! Zstandard coding (`zstd`).
        Zstd,
        //! Dictionary-compressed Brotli coding (`dcb`).
        Dcb,
        //! Dictionary-compressed Zstandard coding (`dcz`).
        Dcz
    };
}

#include <Thoth/Http/NHeaders/Headers/ContentEncodingEnum.tpp>

static_assert(Thoth::Utils::Serializable<Thoth::Http::NHeaders::ContentEncodingEnum>);
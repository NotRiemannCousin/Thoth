#pragma once
#include <Thoth/Http/NHeaders/_base/Weighted.hpp>

namespace Thoth::Http::NHeaders {
    //! @brief Content codings that a recipient may advertise in `Accept-Encoding`.
    //!
    //! The values correspond to RFC 9110 §12.5.3 coding tokens. The enum is wrapped by AcceptEncoding, which also
    //! stores the preference weight.
    enum class AcceptEncodingEnum {
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
        Dcz,
        //! No content coding (`identity`).
        Identity,
        //! Wildcard token (`*`), matching any available coding.
        Wildcard
    };
}

#include <Thoth/Http/NHeaders/Headers/AcceptEncoding.tpp>

namespace Thoth::Http::NHeaders {
    //! @brief Weighted version of `AcceptEncodingEnum`.
    //! @copydoc AcceptEncodingEnum
    using AcceptEncoding = Weighted<AcceptEncodingEnum>;

    static_assert(Thoth::Utils::Serializable<AcceptEncoding>);
}
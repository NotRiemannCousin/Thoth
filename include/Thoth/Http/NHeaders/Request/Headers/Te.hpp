#pragma once
#include <Thoth/Http/NHeaders/_base/Weighted.hpp>

namespace Thoth::Http::NHeaders {
    //! @brief Transfer codings a client is willing to accept in `TE` (RFC 9112 §7.4).
    //!
    //! The `Trailers` value represents the special `trailers` token. Values are wrapped by `Te`, which also carries an
    //! optional quality weight.
    enum class TeEnum {
        //! `compress` transfer coding.
        Compress,
        //! `deflate` transfer coding.
        Deflate,
        //! `gzip` transfer coding.
        Gzip,
        //! Permission to receive trailer fields.
        Trailers
    };
}

#include <Thoth/Http/NHeaders/Request/Headers/Te.tpp>

namespace Thoth::Http::NHeaders {
    //! @brief Weighted version of `TeEnum`.
    //! @copydoc TeEnum
    using Te = Weighted<TeEnum>;

    static_assert(Thoth::Utils::Serializable<Te>);
}

#pragma once
#include <Thoth/Http/NHeaders/_base/Weighted.hpp>

namespace Thoth::Http::NHeaders {
    //! @brief Supported content coding values for the Accept-Encoding header (RFC 9110 §12.5.3).
    enum class AcceptEncodingEnum {
        Gzip,
        Compress,
        Deflate,
        Br,
        Zstd,
        Dcb,
        Dcz,
        Identity,
        Wildcard
    };
}

#include <Thoth/Http/NHeaders/Headers/AcceptEncodingEnum.tpp>

namespace Thoth::Http::NHeaders {
    using AcceptEncoding = Weighted<AcceptEncodingEnum>;

    static_assert(Thoth::Utils::Serializable<AcceptEncoding>);
}
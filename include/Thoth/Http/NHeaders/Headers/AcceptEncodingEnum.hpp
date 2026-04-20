#pragma once

namespace Thoth::Http::NHeaders {
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

static_assert(Thoth::Utils::Serializable<Thoth::Http::NHeaders::AcceptEncodingEnum>);
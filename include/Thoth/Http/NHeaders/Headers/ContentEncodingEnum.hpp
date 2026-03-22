#pragma once

namespace Thoth::Http::NHeaders {
    enum class ContentEncodingEnum {
        Gzip,
        Compress,
        Deflate,
        Br,
        Zstd,
        Dcb,
        Dcz
    };
}

#include <Thoth/Http/NHeaders/Headers/ContentEncodingEnum.tpp>
#pragma once

namespace Thoth::Http::NHeaders {
    enum class TransferEncodingEnum {
        Chunked,
        Compress,
        Deflate,
        Gzip
    };
}

#include <Thoth/Http/NHeaders/Headers/TransferEncodingEnum.tpp>
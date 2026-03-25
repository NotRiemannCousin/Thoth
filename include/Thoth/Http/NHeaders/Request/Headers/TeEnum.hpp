#pragma once

namespace Thoth::Http::NHeaders {
    enum class TeEnum {
        Compress,
        Deflate,
        Gzip,
        Trailers
    };
}

#include <Thoth/Http/NHeaders/Request/Headers/TeEnum.tpp>
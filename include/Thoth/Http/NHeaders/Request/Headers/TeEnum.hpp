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

static_assert(Thoth::Utils::Serializable<Thoth::Http::NHeaders::TeEnum>);
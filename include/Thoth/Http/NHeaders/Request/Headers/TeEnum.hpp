#pragma once
#include <Thoth/Http/NHeaders/_base/Weighted.hpp>

namespace Thoth::Http::NHeaders {
    enum class TeEnum {
        Compress,
        Deflate,
        Gzip,
        Trailers
    };
}

#include <Thoth/Http/NHeaders/Request/Headers/TeEnum.tpp>

namespace Thoth::Http::NHeaders {
    using Te = Weighted<TeEnum>;

    static_assert(Thoth::Utils::Serializable<Te>);
}

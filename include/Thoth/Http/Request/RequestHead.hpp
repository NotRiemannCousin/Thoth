#pragma once
#include <Thoth/Http/NHeaders/Request/RequestHeaders.hpp>
#include <Thoth/Http/Url/Url.hpp>
#include <Thoth/Http/_base.hpp>

namespace Thoth::Http {
    struct RequestHead {
        Url            url;
        VersionEnum    version{ VersionEnum::HTTP1_1 };
        RequestHeaders headers{ Headers::DefaultHeaders() };
    };
}

#include <Thoth/Http/Request/RequestHead.tpp>
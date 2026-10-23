#pragma once
#include <Thoth/Http/NHeaders/Request/RequestHeaders.hpp>
#include <Thoth/Http/_base/VersionEnum.hpp>
#include <Thoth/Http/Url/Url.hpp>

namespace Thoth::Http {
    struct RequestHead {
        Url            url{ Url::Placeholder() };
        VersionEnum    version{ VersionEnum::Http1_1 };
        RequestHeaders headers{ Headers::DefaultHeaders() };
    };
}

#include <Thoth/Http/Request/RequestHead.tpp>
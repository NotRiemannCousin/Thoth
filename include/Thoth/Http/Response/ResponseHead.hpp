#pragma once
#include <string>

#include <Thoth/Http/NHeaders/Response/ResponseHeaders.hpp>
#include <Thoth/Http/Response/StatusCodeEnum.hpp>
#include <Thoth/Http/_base/VersionEnum.hpp>


namespace Thoth::Http {
    struct ResponseHead {
        VersionEnum     version{ VersionEnum::Http1_1 };
        StatusCodeEnum  status{ StatusCodeEnum::Ok };
        ResponseHeaders headers{ Headers::DefaultHeaders() };
    };
}

#include  <Thoth/Http/Response/ResponseHead.tpp>
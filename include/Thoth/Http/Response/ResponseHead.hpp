#pragma once
#include <string>

#include <Thoth/Http/NHeaders/Response/ResponseHeaders.hpp>
#include <Thoth/Http/Response/StatusCodeEnum.hpp>


namespace Thoth::Http {
    struct ResponseHead {
        VersionEnum     version{ VersionEnum::HTTP1_1 };
        StatusCodeEnum  status{ StatusCodeEnum::Ok };
        std::string     statusMessage{};
        ResponseHeaders headers{ Headers::DefaultHeaders() };
    };
}

#include  <Thoth/Http/Response/ResponseHead.tpp>
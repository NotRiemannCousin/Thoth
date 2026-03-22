#pragma once
#include <Thoth/Http/Response/StatusCodeEnum.hpp>
#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <string>


namespace Thoth::Http {
    struct ResponseHead {
        VersionEnum version{};
        StatusCodeEnum status{};
        std::string statusMessage{};
        Headers headers{};
    };
}

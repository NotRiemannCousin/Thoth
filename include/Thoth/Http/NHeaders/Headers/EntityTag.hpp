#pragma once
#include <string>

namespace Thoth::Http::NHeaders {
    struct EntityTag {
        std::string tag;
        bool isWeak;
    };
}

#include <Thoth/Http/NHeaders/Headers/EntityTag.tpp>
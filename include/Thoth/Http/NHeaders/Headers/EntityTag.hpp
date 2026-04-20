#pragma once
#include <string>

namespace Thoth::Http::NHeaders {
    struct EntityTag {
        std::string tag;
        bool isWeak;
    };
}

#include <Thoth/Http/NHeaders/Headers/EntityTag.tpp>

static_assert(Thoth::Utils::Serializable<Thoth::Http::NHeaders::EntityTag>);
#pragma once
#include <string>
#include <optional>

namespace Thoth::Http::NHeaders {
    struct Upgrade {
        std::string protocol{};
        std::optional<std::string> version{};
    };
}

#include <Thoth/Http/NHeaders/Headers/Upgrade.tpp>
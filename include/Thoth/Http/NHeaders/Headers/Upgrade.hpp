#pragma once
#include <string>
#include <optional>

namespace Thoth::Http::NHeaders {
    struct Upgrade {
        std::string protocol{};
        std::optional<std::string> version{};

        bool operator==(const Upgrade& rhs) const noexcept = default;
    };
}

#include <Thoth/Http/NHeaders/Headers/Upgrade.tpp>

static_assert(Thoth::Utils::Serializable<Thoth::Http::NHeaders::Upgrade>);
#pragma once
#include <string>
#include <Thoth/Http/NHeaders/_base.hpp>
#include <Thoth/Http/NHeaders/_base/Parameterized.hpp>

namespace Thoth::Http::NHeaders {
    struct LinkHeader {
        std::string uri{};

        bool operator==(const LinkHeader&) const = default;
    };
}

#include <Thoth/Http/NHeaders/Headers/Link.tpp>

namespace Thoth::Http::NHeaders {
    using Link = Parameterized<LinkHeader>;

    static_assert(Thoth::Utils::Serializable<LinkHeader>);
}
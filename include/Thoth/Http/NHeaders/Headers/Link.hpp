#pragma once
#include <string>
#include <Thoth/Http/NHeaders/_base.hpp>
#include <Thoth/Http/NHeaders/_base/Parameterized.hpp>

namespace Thoth::Http::NHeaders {
    //! @brief The URI portion of one `Link` field value (RFC 8288 §3.1).
    //!
    //! The wire representation encloses this URI in angle brackets. Link parameters such as `rel`, `type` and
    //! `hreflang` are stored by the `Link` wrapper.
    //!
    //! @par Example
    //! @code{.cpp}
    //! LinkHeader target{ "https://example.test/next" };
    //! @endcode
    struct LinkHeader {
        std::string uri{};

        bool operator==(const LinkHeader&) const = default;
    };
}

#include <Thoth/Http/NHeaders/Headers/Link.tpp>

namespace Thoth::Http::NHeaders {
    //! @brief Parameterized version of `LinkHeader`.
    //! @copydoc LinkHeader
    using Link = Parameterized<LinkHeader>;

    static_assert(Thoth::Utils::Serializable<LinkHeader>);
}
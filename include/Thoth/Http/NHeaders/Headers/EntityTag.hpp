#pragma once
#include <string>

namespace Thoth::Http::NHeaders {
    //! @brief An HTTP entity tag used for cache validation (RFC 9110 §8.8.3).
    //!
    //! `isWeak` distinguishes a weak validator (`W/"tag"`) from a strong validator (`"tag"`). The `tag` member stores
    //! the opaque tag content without the surrounding quotes or the optional `W/` prefix.
    //!
    //! @par Example
    //! @code{.cpp}
    //! EntityTag tag{ "0815", true }; // formats as W/"0815"
    //! @endcode
    struct EntityTag {
        //! Opaque entity-tag content without quotes.
        std::string tag;
        //! Whether this is a weak validator.
        bool isWeak;
    };
}

#include <Thoth/Http/NHeaders/Headers/EntityTag.tpp>

static_assert(Thoth::Utils::Serializable<Thoth::Http::NHeaders::EntityTag>);
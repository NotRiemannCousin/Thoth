#pragma once
#include <optional>
#include <variant>

namespace Thoth::Http::NHeaders {
    //! @brief A byte range with an explicit starting offset (RFC 9110 §14.2).
    //!
    //! `end` is present for a closed range such as `bytes=200-1000` and absent for an open-ended range such as
    //! `bytes=500-`.
    //!
    //! @par Example
    //! @code{.cpp}
    //! PrefixedRange range{ 200u, 1000u }; // bytes=200-1000
    //! @endcode
    struct PrefixedRange {
        //! First requested byte offset.
        unsigned int start;
        //! Optional inclusive last byte offset.
        std::optional<unsigned int> end; // make start + count to eliminate invalid state?
    };

    //! @brief A suffix byte range requesting the last `last` bytes (RFC 9110 §14.2).
    //!
    //! This represents the `bytes=-42` form of the HTTP `Range` header.
    //! @par Example
    //! @code{.cpp}
    //! SuffixedRange range{ 42u }; // bytes=-42
    //! @endcode
    struct SuffixedRange {
        //! Number of bytes requested from the end of the representation.
        unsigned int last;
    };

    //! @brief The supported byte-range forms of the HTTP `Range` header (RFC 9110 §14.2).
    //!
    //! A `Range` is either a `PrefixedRange` (`bytes=start-end` or
    //! `bytes=start-`) or a `SuffixedRange` (`bytes=-suffix`).
    //! @par Example
    //! @code{.cpp}
    //! Range range = PrefixedRange{ 500u, std::nullopt }; // bytes=500-
    //! @endcode
    using Range = std::variant<PrefixedRange, SuffixedRange>;
}


#include <Thoth/Http/NHeaders/Request/Headers/Range.tpp>

static_assert(Thoth::Utils::Serializable<Thoth::Http::NHeaders::Range>);
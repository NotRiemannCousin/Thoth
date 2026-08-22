#pragma once
#include <string>
#include <Thoth/Http/NHeaders/_base.hpp>
#include <Thoth/Http/NHeaders/_base/Parameterized.hpp>

namespace Thoth::Http::NHeaders {
    //! @brief The disposition-type token of a `Content-Disposition` header (RFC 6266 §4).
    //!
    //! Typical values are `attachment` and `inline`. The scanner stores the token lowercased because the disposition
    //! type is case-insensitive.
    //! Parameters such as `filename` belong to `ContentDisposition`.
    //! @par Example
    //! @code{.cpp}
    //! DispositionTypeHeader disposition{ "attachment" };
    //! @endcode
    struct DispositionTypeHeader {
        //! Lowercase disposition token.
        std::string type{};

        bool operator==(const DispositionTypeHeader&) const = default;
    };
}

#include <Thoth/Http/NHeaders/Response/Headers/ContentDisposition.tpp>

namespace Thoth::Http::NHeaders {
    //! @brief Parameterized version of `DispositionTypeHeader`.
    //! @copydoc DispositionTypeHeader
    using ContentDisposition = Parameterized<DispositionTypeHeader>;

    static_assert(Thoth::Utils::Serializable<DispositionTypeHeader>);
}
#pragma once
#include <Thoth/Http/Middleware/_base.hpp>
#include <Thoth/Http/Client/Client.hpp>

namespace Thoth::Http {
    //! @brief Wraps a handler to reverse `Content-Encoding` on the response body (RFC 9110 §8.4). Undoes codings in
    //! reverse application order.
    //!
    //! - Supported: `gzip` and `deflate` (via zlib).
    //! - Unsupported: `br` and `zstd` (returns an error as no codec is linked yet).
    //!
    //! @note Scoped strictly to `Body = std::string`. Decompressing directly into an arbitrary streaming sink would
    //! need the codec itself to stream, which this doesn't do.
    //! @tparam Next Inner handler.
    template<class Next>
    auto Decompress(Next next);
}

#include <Thoth/Http/Middleware/Decompress.tpp>

namespace Thoth::Http {
    static_assert(MiddlewareConcept<
        decltype([](auto next) { return Decompress(std::move(next)); }),
        GetMethod, GetMethod, std::string
    >);
}
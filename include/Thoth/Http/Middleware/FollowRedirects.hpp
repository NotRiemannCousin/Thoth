#pragma once
#include <Thoth/Http/Middleware/_base.hpp>
#include <Thoth/Http/Client/Client.hpp>

namespace Thoth::Http {
    //! @brief Wraps a handler so `307` and `308` responses are followed automatically.
    //!
    //! - Never changes `Method` or `Body` (RFC 9110 §15.4.8-9 forbids it for these two codes).
    //! - Works as a generic template-lambda: the returned handler is reusable for any `Method`/`Body`.
    //!
    //! @see docs/pages/Middleware.md for behavior details and composition order.
    //! @see Thoth::Http::StatusCodeEnum for more info.
    //!
    //! @tparam Next Inner handler.
    //! @param next Handler invoked on the first attempt and on every hop.
    //! @param maxHops Maximum redirects followed before giving up.
    template<class Next>
    auto FollowRedirects(Next next, uint32_t maxHops = 10);
}

#include <Thoth/Http/Middleware/FollowRedirects.tpp>

namespace Thoth::Http {
    static_assert(MiddlewareConcept<
        decltype([](auto next) { return FollowRedirects(std::move(next)); }),
        GetMethod, GetMethod, std::string
    >);
}
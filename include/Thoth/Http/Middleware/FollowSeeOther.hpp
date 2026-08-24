#pragma once
#include <Thoth/Http/Middleware/_base.hpp>
#include <Thoth/Http/Client/Client.hpp>
#include <Thoth/Http/Methods/HeadMethod.hpp>

namespace Thoth::Http {
    //! @brief The HTTP method a request is left with after a `303 See Other` (RFC 9110 §15.4.4):
    //!
    //! - `GetMethod` and `HeadMethod` are preserved.
    //! - Everything else downgrades to `GetMethod`.
    //! - Pure function of `M` — no runtime data needed, so no `std::variant` is required.
    template<MethodConcept M>
    using SeeOtherMethod = std::conditional_t<std::same_as<M, GetMethod> || std::same_as<M, HeadMethod>, M, GetMethod>;

    //! @brief Wraps a handler so a `303 See Other` response is followed. Downgrades to @ref SeeOtherMethod and drops
    //! the body, per RFC 9110 §15.4.4.
    //!
    //! @note `next` must satisfy `HandlerConcept` for *both* the original `Method` and `SeeOtherMethod<Method>`.
    //! Trivially true for generic handlers like `Client::H_Send`, but false for handlers fixed to a concrete `Method`.
    //! @tparam Next Inner handler.
    //! @param next Handler invoked with the original request, then with the downgraded one.
    //! @param maxHops Maximum further 303 hops followed once already downgraded.
    template<class Next>
    auto FollowSeeOther(Next next, uint32_t maxHops = 10);
}

#include <Thoth/Http/Middleware/FollowSeeOther.tpp>

namespace Thoth::Http {
    static_assert(MiddlewareConcept<
        decltype([](auto next) { return FollowSeeOther(std::move(next)); }),
        PostMethod, GetMethod, std::string
    >); // type-changing: PostMethod -> GetMethod (like 303)
}
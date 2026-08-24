#pragma once
#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/Client/Client.hpp>
#include <Thoth/Http/Methods/GetMethod.hpp>
#include <Thoth/ThothError.hpp>

#include <concepts>
#include <utility>

namespace Thoth::Http {
    //! @brief Defines a callable that processes a request and produces a response.
    //! Matches the signature `[](Request<Method, Body>) -> Client::ExpResponse<Method, Body>`.
    //!
    //! This is like @ref Thoth::Http::Client::H_Send "Client::H_Send()", you transfora a
    //! @ref Thoth::Http::Request "Request" into a `std::expected` @ref Thoth::Http::Response "Response". The
    //! @ref Thoth::Http::Client "Client::Send" method family and the result type of these middlewares models it.
    //!
    //! @see Thoth::Http::MiddlewareConcept "MiddlewareConcept" to know more about middlewares.
    template<class H, class Method, class Body>
    concept HandlerConcept = MethodConcept<Method> && BodyConcept<Body> &&
        requires(H handler, Request<Method, Body> request) {
            { handler(std::move(request)) } -> std::same_as<Client::ExpResponse<Method, Body>>;
        };

    namespace details_ {
        //! @brief A dummy handler used exclusively to test middleware factories in MiddlewareConcept.
        //! Calling it always returns an error.
        template<BodyConcept Body>
        inline constexpr auto nullHandler{ []<MethodConcept M>(Request<M, Body>) -> Client::ExpResponse<M, Body> {
            return ThothUnex{ GenericError{ "NullHandler must never actually be called" } };
        } };

        //! @brief A dummy middleware that returns the handler unchanged. Used to verify MiddlewareConcept.
        inline constexpr auto identityMiddleware{ [](auto next) { return next; } };
    }

    //! @brief Defines a factory that takes an inner @ref Thoth::Http::HandlerConcept "handler" and returns a new
    //! wrapped handler.
    //!
    //! Given this constraint you can chain multiple middlewares because the return handler connects with the input
    //! from another.
    // TODO: Add example
    //!
    //! It verifies the handler-to-handler transformation:
    //! - Type-preserving: `InMethod` matches `OutMethod` (e.g., FollowRedirects).
    //! - Type-changing: `InMethod` differs from `OutMethod` (e.g., FollowSeeOther drops to GetMethod).
    //!
    //! @note Function templates cannot be directly checked by concepts. A wrapping lambda is required.
    //! @note You can create a function that returns a middleware lambda, this is in fact what every middleware here
    //! do, so you can configure the behaviour like retry count and proxy info.
    template<class M, class InMethod, class OutMethod, class Body>
    concept MiddlewareConcept =
        MethodConcept<InMethod> && MethodConcept<OutMethod> && BodyConcept<Body> &&
        requires(M middleware) {
            { middleware(details_::nullHandler<Body>) } -> HandlerConcept<OutMethod, Body>;
        };

    static_assert(HandlerConcept<decltype(details_::nullHandler<std::string>), GetMethod, std::string>);
    static_assert(MiddlewareConcept<decltype(details_::identityMiddleware), GetMethod, GetMethod, std::string>);
}
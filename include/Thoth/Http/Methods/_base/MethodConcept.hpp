#pragma once
#include <Thoth/Http/Response/StatusCodeEnum.hpp>
#include <Thoth/Http/Request/Url.hpp>
#include <Thoth/Http/Headers.hpp>

#include <string_view>


namespace Thoth::Http {

    //! @brief The HTTP methods requirements following  <a href="https://datatracker.ietf.org/doc/html/rfc9110#method.extensibility">
    //! RFC9110</a>.
    //!
    //! Canonical methods of HTTP just compare if a request is well-formed with body, but some methods (like
    //! "Lock-Token" for LOCK / UNLOCK in WebDAV or RPC's "Content-Type: application/rpc") also needs to validate
    //! the headers, so I decided to leave it more generic so a class that satisfies MethodConcept have all the
    //! info available to do it validation.
    template<class T>
    concept MethodConcept = requires(T t, string_view body, Url url, Headers headers, StatusCodeEnum statusCode) {
        requires requires { { T::MethodName() } -> std::same_as<string_view>; };

        { T::IsSafe()       } -> std::same_as<bool>;
        { T::IsIdempotent() } -> std::same_as<bool>;

        { T::ValidateRequest(body, url, headers)   } -> std::same_as<WebResultOper>;
        { T::ValidateResponse(statusCode, body, url, headers)  } -> std::same_as<WebResultOper>;
    };
}

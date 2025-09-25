#pragma once
#include <Thoth/Http/HttpStatusCodeEnum.hpp>
#include <Thoth/Http/HttpHeaders.hpp>
#include <Thoth/Http/HttpUrl.hpp>

#include <string_view>


namespace Thoth::Http {

    //! @brief The HTTP methods requirements following  <a href="https://datatracker.ietf.org/doc/html/rfc9110#method.extensibility">
    //! RFC9110</a>.
    //!
    //! Canonical methods of HTTP just compare if a request is well-formed with body, but some methods (like
    //! "Lock-Token" for LOCK / UNLOCK in WebDAV or RPC's "Content-Type: application/rpc") also needs to validate
    //! the headers, so I decided to leave it more generic so a class that satisfies HttpMethodConcept have all the
    //! info available to do it validation.
    template<class T>
    concept HttpMethodConcept = requires(T t, string_view body, HttpUrl url, HttpHeaders headers, HttpStatusCodeEnum statusCode) {
        requires requires { { T::MethodName() } -> std::same_as<string_view>; };

        { T::IsSafe()       } -> std::same_as<bool>;
        { T::IsIdempotent() } -> std::same_as<bool>;

        { T::ValidateRequest(body, url, headers)   } -> std::same_as<WebResultOper>;
        { T::ValidateResponse(statusCode, body, url, headers)  } -> std::same_as<WebResultOper>;
    };
}

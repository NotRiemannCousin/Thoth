#pragma once
#include <Thoth/Http/NHeaders/Headers.hpp>

#include <Thoth/Http/NHeaders/Request/Headers/_pch.hpp>
#include <Thoth/Http/NHeaders/Headers/EntityTag.hpp>

namespace Thoth::Http {
    // based in Microslop's HttpRequestHeaders
    //! @brief Represents the request-specific HTTP header fields.
    //!
    //! The typed accessors use concrete models such as `Range`, `EntityTag`, `Te`, `FreeWeightedHeader` and
    //! `FreeParameterizedHeader`.
    //!
    //! @par Example
    //! @code{.cpp}
    //! RequestHeaders headers;
    //! headers.Host().Set("example.test");
    //! headers.Range().Add(NHeaders::Range{
    //!     NHeaders::PrefixedRange{ 0u, 1023u } });
    //! auto host{ headers.Host().Get() };
    //! @endcode
    struct RequestHeaders : Headers {
        //! @copydoc Headers::Parse
        template<std::ranges::input_range R>
        static std::expected<RequestHeaders, MessageParseErrorEnum> Parse(R&& headers, size_t maxHeadersLength = 1<<16);

#pragma region Raw and Collection Views

        //! @name Raw and Collection Views
        //! Methods for accessing headers without immediate parsing or as raw collections.
        //! @{

        //! @brief Gets a readonly view of all Cookie values (name/value pairs, no attributes).
        //! @warning Risk of dangling reference if the underlying collection is modified.
        [[nodiscard]] auto GetCookiesView() const;

        //! @}
#pragma endregion



#pragma region Request Specific Proxies
        //! @name Request Specific Proxies
        //! @{
        //! Convenient calls to some headers.

        //! @brief Gets the Authorization header (e.g., Bearer, Basic).
        //! @par Example
        //! @code{.cpp}
        //! auto authorization{ headers.Authorization().GetAsOpt() }; // string
        //! @endcode
        NHeaders::ValueProxy<false, std::string> Authorization();
        //! @copydoc Authorization
        [[nodiscard]] NHeaders::ValueProxy<true, std::string> Authorization() const;

        //! @brief Gets the Host header. Essential for HTTP/1.1 and SNI.
        //!
        //! @note Can be a relative URL, resolves via @ref Url::Resolve using a proper URL.
        //! @par Example
        //! @code{.cpp}
        //! headers.Host().Set("example.test:443");
        //! auto host{ headers.Host().Get() };
        //! @endcode
        NHeaders::ValueProxy<false, std::string> Host();
        //! @copydoc Host
        [[nodiscard]] NHeaders::ValueProxy<true, std::string> Host() const;

        //! @brief The value of the Referer header (yes, it's misspelled in RFC).
        //!
        //! @note Can be a relative URL, resolves via @ref Url::Resolve using a proper URL.
        //! @par Example
        //! @code{.cpp}
        //! headers.Referrer().TrySet("https://origin.example/doc");
        //! @endcode
        NHeaders::ValueProxy<false, std::string> Referrer();
        //! @copydoc Referrer
        [[nodiscard]] NHeaders::ValueProxy<true, std::string> Referrer() const;

        //! @brief The value of the Origin header. Used for CORS requests.
        //!
        //! @note Can be a relative URL, resolves via @ref Url::Resolve using a proper URL.
        //! @par Example
        //! @code{.cpp}
        //! headers.Origin().TrySet("https://app.example");
        //! @endcode
        NHeaders::ValueProxy<false, std::string, std::monostate> Origin();
        //! @copydoc Origin
        [[nodiscard]] NHeaders::ValueProxy<true, std::string, std::monostate> Origin() const;

        //! @brief The value of the From header (email address of the user).
        //! @par Example
        //! @code{.cpp}
        //! auto from{ headers.From().GetAsOpt() }; // string
        //! @endcode
        NHeaders::ValueProxy<false, std::string> From();
        //! @copydoc From
        [[nodiscard]] NHeaders::ValueProxy<true, std::string> From() const;

        //! @brief The value of the Max-Forwards header for limiting proxy hops.
        //! @par Example
        //! @code{.cpp}
        //! headers.MaxForwards().Set(5u);
        //! auto hops{ headers.MaxForwards().GetWithDefault(0u) };
        //! @endcode
        NHeaders::ValueProxy<false, unsigned int> MaxForwards();
        //! @copydoc MaxForwards
        [[nodiscard]] NHeaders::ValueProxy<true, unsigned int> MaxForwards() const;

        // //! @brief The value of the :protocol pseudo-header (primarily for HTTP/2/3).
        // void GetProtocol();
        // //! @copydoc GetProtocol
        // [[nodiscard]] void GetProtocol() const;

        //! @brief The value of the Proxy-Authorization header.
        //! @par Example
        //! @code{.cpp}
        //! headers.ProxyAuthorization().TrySet("Basic dXNlcjpwYXNz");
        //! @endcode
        NHeaders::ValueProxy<false, std::string> ProxyAuthorization();
        //! @copydoc ProxyAuthorization
        [[nodiscard]] NHeaders::ValueProxy<true, std::string> ProxyAuthorization() const;

        //! @brief The value of the Range header for partial content requests.
        //! @par Example
        //! @code{.cpp}
        //! headers.Range().Add(NHeaders::Range{
        //!     NHeaders::PrefixedRange{ 0u, 1023u } });
        //! @endcode
        NHeaders::ListProxy<false, NHeaders::Range> Range();
        //! @copydoc Range
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::Range> Range() const;

        // //! @brief The value of the Cache-Control header for the request.
        // NHeaders::ValueProxy<false, std::string> CacheControl();
        // //! @copydoc CacheControl
        // [[nodiscard]] NHeaders::ValueProxy<true, std::string> CacheControl() const;

        //! @brief The value of the If-Modified-Since header.
        //! @par Example
        //! @code{.cpp}
        //! auto since{ headers.IfModifiedSince().GetAsOpt() }; // utc_clock::time_point
        //! @endcode
        NHeaders::ValueProxy<false, std::chrono::utc_clock::time_point> IfModifiedSince();
        //! @copydoc IfModifiedSince
        [[nodiscard]] NHeaders::ValueProxy<true, std::chrono::utc_clock::time_point> IfModifiedSince() const;

        //! @brief The value of the If-Unmodified-Since header.
        //! @par Example
        //! @code{.cpp}
        //! auto since{ headers.IfUnmodifiedSince().GetAsOpt() }; // utc_clock::time_point
        //! @endcode
        NHeaders::ValueProxy<false, std::chrono::utc_clock::time_point> IfUnmodifiedSince();
        //! @copydoc IfUnmodifiedSince
        [[nodiscard]] NHeaders::ValueProxy<true, std::chrono::utc_clock::time_point> IfUnmodifiedSince() const;

        //! @brief The value of the If-Range header.
        //! @par Example
        //! @code{.cpp}
        //! auto range{ headers.IfRange().GetAsOpt() };
        //! @endcode
        NHeaders::ValueProxy<false, std::chrono::utc_clock::time_point, std::string> IfRange();
        //! @copydoc IfRange
        [[nodiscard]] NHeaders::ValueProxy<true, std::chrono::utc_clock::time_point, std::string> IfRange() const;

        //! @brief The value of the If-Match header.
        //! @par Example
        //! @code{.cpp}
        //! headers.IfMatch().Add(NHeaders::EntityTag{ "0815", false });
        //! @endcode
        NHeaders::ListProxy<false, NHeaders::EntityTag> IfMatch();
        //! @copydoc IfMatch
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::EntityTag> IfMatch() const;

        //! @brief The value of the If-None-Match header.
        //! @par Example
        //! @code{.cpp}
        //! headers.IfNoneMatch().Set(std::vector<NHeaders::EntityTag>{ { "0815", false }, { "cached", true } });
        //! @endcode
        NHeaders::ListProxy<false, NHeaders::EntityTag> IfNoneMatch();
        //! @copydoc IfNoneMatch
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::EntityTag> IfNoneMatch() const;
        //! @}


        //! @brief The value of the Accept-Language header, with an optional preference weight.
        //! @par Example
        //! @code{.cpp}
        //! auto portuguese{ NHeaders::FreeWeightedHeader{ "pt-BR" } };
        //! headers.AcceptLanguage().Add(portuguese.WithWeight(0.9));
        //! @endcode
        NHeaders::ListProxy<false, NHeaders::FreeWeightedHeader> AcceptLanguage();
        //! @copydoc AcceptLanguage
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::FreeWeightedHeader> AcceptLanguage() const;

        //! @brief The value of the TE (Transfer Encoding) header, with an optional preference weight.
        //! @note Per RFC 9110 §10.1.4, "trailers" itself doesn't take a weight, this doesn't enforce that.
        //! @par Example
        //! @code{.cpp}
        //! headers.Te().Add(NHeaders::Te{ NHeaders::TeEnum::Gzip }.WithWeight(0.5));
        //! @endcode
        NHeaders::ListProxy<false, NHeaders::Te> Te();
        //! @copydoc Te
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::Te> Te() const;



        //! @brief The value of the Prefer header, with.
        //! @par Example
        //! @code{.cpp}
        //! auto async{ NHeaders::FreeParameterizedHeader{ "respond-async" }
        //!     .WithParam("wait", "10") };
        //! headers.Prefer().Add(async);
        //! @endcode
        NHeaders::ListProxy<false, NHeaders::FreeParameterizedHeader> Prefer();
        //! @copydoc Prefer
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::FreeParameterizedHeader> Prefer() const;

        //! @brief The value of the Expect header.
        //! @par Example
        //! @code{.cpp}
        //! headers.Expect().Add(
        //!     NHeaders::FreeParameterizedHeader{ "100-continue" });
        //! @endcode
        NHeaders::ListProxy<false, NHeaders::FreeParameterizedHeader> Expect();
        //! @copydoc Expect
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::FreeParameterizedHeader> Expect() const;
        //! @}
#pragma endregion

// #pragma region CORS Request Headers
//         //! @name CORS Request Headers
//         //! Headers sent by the client during Cross-Origin Resource Sharing.
//         //! @{
//
//         //! @brief The value of the Access-Control-Request-Headers for preflight requests.
//         NHeaders::ListProxy<false, std::string>NHeaders::V<alueProxy<, std::string> AccessControlAllowCredentials();
//         //! @copydoc AccessControlAllowCredentials
//         [[nodiscard]] NHeaders::ListProxy<true, std::string>NHeaders::ValueProxytruefalse, std::string> AccessControlAllowCredentials() const;
//
//         //! @brief The value of the Access-Control-Request-Headers for preflight requests.
//         NHeaders::ValueProxy<false, std::string> AccessControlAllowHeaders();
//         //! @copydoc AccessControlAllowHeaders
//         [[nodiscard]] NHeaders::ValueProxy<true, std::string> AccessControlAllowHeaders() const;
//
//         //! @brief The value of the Access-Control-Request-Headers for preflight requests.
//         NHeaders::ValueProxy<false, std::string> AccessControlAllowMethods();
//         //! @copydoc AccessControlAllowMethods
//         [[nodiscard]] NHeaders::ValueProxy<true, std::string> AccessControlAllowMethods() const;
//
//         //! @brief The value of the Access-Control-Request-Headers for preflight requests.
//         NHeaders::ValueProxy<false, std::string> AccessControlAllowOrigin();
//         //! @copydoc AccessControlAllowOrigin
//         [[nodiscard]] NHeaders::ValueProxy<true, std::string> AccessControlAllowOrigin() const;
//
//
//         //! @brief The value of the Access-Control-Request-Headers for preflight requests.
//         NHeaders::ValueProxy<false, std::string> AccessControlExposeAge();
//         //! @copydoc AccessControlExposeAge
//         [[nodiscard]] NHeaders::ValueProxy<true, std::string> AccessControlExposeAge() const;
//
//         //! @brief The value of the Access-Control-Request-Headers for preflight requests.
//         NHeaders::ValueProxy<false, std::string> AccessControlExposeMethods();
//         //! @copydoc AccessControlExposeMethods
//         [[nodiscard]] NHeaders::ValueProxy<true, std::string> AccessControlExposeMethods() const;
//
//
//         //! @brief The value of the Access-Control-Request-Method for preflight requests.
//         NHeaders::ValueProxy<false, std::string> AccessControlRequestMethod();
//         //! @copydoc AccessControlRequestMethod
//         [[nodiscard]] NHeaders::ValueProxy<true, std::string> AccessControlRequestMethod() const;
//
//         //! @brief The value of the Access-Control-Request-Headers for preflight requests.
//         NHeaders::ValueProxy<false, std::string> AccessControlRequestHeaders();
//         //! @copydoc AccessControlRequestHeaders
//         [[nodiscard]] NHeaders::ValueProxy<true, std::string> AccessControlRequestHeaders() const;
//         //! @}
// #pragma endregion
    };
};

#include <Thoth/Http/NHeaders/Request/RequestHeaders.tpp>
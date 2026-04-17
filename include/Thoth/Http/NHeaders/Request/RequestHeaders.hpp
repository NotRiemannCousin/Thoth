#pragma once
#include <Thoth/Http/NHeaders/Headers.hpp>

#include <Thoth/Http/NHeaders/Request/Headers/_pch.hpp>
#include <Thoth/Http/NHeaders/Headers/EntityTag.hpp>

namespace Thoth::Http {
    // based in Microslop's HttpRequestHeaders

    //! @brief Represents the collection of HTTP headers associated with a request.
    //! Based on modern HTTP RFCs and inspired by .NET's HttpRequestHeaders.
    struct RequestHeaders : Headers {

#pragma region Raw and Collection Views
        //! @name Raw and Collection Views
        //! Methods for accessing headers without immediate parsing or as raw collections.
        //! @{

        //! @brief Gets a view of the contents of this headers collection that does
        //! not parse nor validate data.
        [[nodiscard]] [[nodiscard]] auto GetNonValidatedView() const;

        //! @brief Gets a readonly view of all Cookie values.
        //! @warning Risk of dangling reference if the underlying collection is modified.
        [[nodiscard]] [[nodiscard]] auto GetCookiesView() const;

        //! @}
#pragma endregion



#pragma region Request Specific Proxies
        //! @name Request Specific Proxies
        //! @{
        //! Convenient calls to some headers.

        // TODO: Implement (someday)    
        // ! @brief Gets the Authorization header (e.g., Bearer, Basic).
        NHeaders::ValueProxy<false, std::string> Authorization();
        [[nodiscard]] NHeaders::ValueProxy<true, std::string> Authorization() const;

        using Url = std::string;

        //! @brief Gets the Host header. Essential for HTTP/1.1 and SNI.
        NHeaders::ValueProxy<false, Url> Host();
        [[nodiscard]] NHeaders::ValueProxy<true, Url> Host() const;

        //! @brief The value of the Referer header (yes, it's misspelled in RFC).
        NHeaders::ValueProxy<false, Url> Referrer();
        [[nodiscard]] NHeaders::ValueProxy<true, Url> Referrer() const;

        //! @brief The value of the Origin header. Used for CORS requests.
        NHeaders::ValueProxy<false, Url, std::monostate> Origin();
        [[nodiscard]] NHeaders::ValueProxy<true, Url, std::monostate> Origin() const;

        //! @brief The value of the From header (email address of the user).
        NHeaders::ValueProxy<false, std::string> From();
        [[nodiscard]] NHeaders::ValueProxy<true, std::string> From() const;

        //! @brief The value of the Max-Forwards header for limiting proxy hops.
        NHeaders::ValueProxy<false, unsigned int> MaxForwards();
        [[nodiscard]] NHeaders::ValueProxy<true, unsigned int> MaxForwards() const;

        ////! @brief The value of the :protocol pseudo-header (primarily for HTTP/2/3).
        //void GetProtocol();
        //[[nodiscard]] void GetProtocol() const;

        //! @brief The value of the Proxy-Authorization header.
        NHeaders::ValueProxy<false, std::string> ProxyAuthorization();
        [[nodiscard]] NHeaders::ValueProxy<true, std::string> ProxyAuthorization() const;

        //! @brief The value of the Range header for partial content requests.
        NHeaders::ListProxy<false, NHeaders::Range> Range();
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::Range> Range() const;

        // //! @brief The value of the Cache-Control header for the request.
        // NHeaders::ValueProxy<false, std::string> CacheControl();
        // [[nodiscard]] NHeaders::ValueProxy<true, std::string> CacheControl() const;

        //! @brief The value of the If-Modified-Since header.
        NHeaders::ValueProxy<false, std::chrono::utc_clock::time_point> IfModifiedSince();
        [[nodiscard]] NHeaders::ValueProxy<true, std::chrono::utc_clock::time_point> IfModifiedSince() const;

        //! @brief The value of the If-Unmodified-Since header.
        NHeaders::ValueProxy<false, std::chrono::utc_clock::time_point> IfUnmodifiedSince();
        [[nodiscard]] NHeaders::ValueProxy<true, std::chrono::utc_clock::time_point> IfUnmodifiedSince() const;

        //! @brief The value of the If-Range header.
        NHeaders::ValueProxy<false, std::chrono::utc_clock::time_point, std::string> IfRange();
        [[nodiscard]] NHeaders::ValueProxy<true, std::chrono::utc_clock::time_point, std::string> IfRange() const;

        //! @brief The value of the If-Match header.
        NHeaders::ListProxy<false, NHeaders::EntityTag> IfMatch();
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::EntityTag> IfMatch() const;

        //! @brief The value of the If-None-Match header.
        NHeaders::ValueProxy<false, NHeaders::EntityTag> IfNoneMatch();
        [[nodiscard]] NHeaders::ValueProxy<true, NHeaders::EntityTag> IfNoneMatch() const;
        //! @}


        //! @brief The value of the Accept-Language header.
        NHeaders::ListProxy<false, std::string> AcceptLanguage();
        [[nodiscard]] NHeaders::ListProxy<true, std::string> AcceptLanguage() const;

        //! @brief The value of the TE (Transfer Encoding) header.
        NHeaders::ListProxy<false, NHeaders::TeEnum> Te();
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::TeEnum> Te() const;

        //! @}
#pragma endregion

// #pragma region CORS Request Headers
//         //! @name CORS Request Headers
//         //! Headers sent by the client during Cross-Origin Resource Sharing.
//         //! @{
//
//         //! @brief The value of the Access-Control-Request-Headers for preflight requests.
//         NHeaders::ListProxy<true, std::string>NHeaders::V<alueProxy<, std::string> AccessControlAllowCredentials();
//         [[nodiscard]] NHeaders::ListProxy<false, std::string>NHeaders::ValueProxytruefalse, std::string> AccessControlAllowCredentials() const;
//
//         //! @brief The value of the Access-Control-Request-Headers for preflight requests.
//         NHeaders::ValueProxy<false, std::string> AccessControlAllowHeaders();
//         [[nodiscard]] NHeaders::ValueProxy<true, std::string> AccessControlAllowHeaders() const;
//
//         //! @brief The value of the Access-Control-Request-Headers for preflight requests.
//         NHeaders::ValueProxy<false, std::string> AccessControlAllowMethods();
//         [[nodiscard]] NHeaders::ValueProxy<true, std::string> AccessControlAllowMethods() const;
//
//         //! @brief The value of the Access-Control-Request-Headers for preflight requests.
//         NHeaders::ValueProxy<false, std::string> AccessControlAllowOrigin();
//         [[nodiscard]] NHeaders::ValueProxy<true, std::string> AccessControlAllowOrigin() const;
//
//
//         //! @brief The value of the Access-Control-Request-Headers for preflight requests.
//         NHeaders::ValueProxy<false, std::string> AccessControlExposeAge();
//         [[nodiscard]] NHeaders::ValueProxy<true, std::string> AccessControlExposeAge() const;
//
//         //! @brief The value of the Access-Control-Request-Headers for preflight requests.
//         NHeaders::ValueProxy<false, std::string> AccessControlExposeMethods();
//         [[nodiscard]] NHeaders::ValueProxy<true, std::string> AccessControlExposeMethods() const;
//
//
//         //! @brief The value of the Access-Control-Request-Method for preflight requests.
//         NHeaders::ValueProxy<false, std::string> AccessControlRequestMethod();
//         [[nodiscard]] NHeaders::ValueProxy<true, std::string> AccessControlRequestMethod() const;
//
//         //! @brief The value of the Access-Control-Request-Headers for preflight requests.
//         NHeaders::ValueProxy<false, std::string> AccessControlRequestHeaders();
//         [[nodiscard]] NHeaders::ValueProxy<true, std::string> AccessControlRequestHeaders() const;
//         //! @}
// #pragma endregion
    };
};
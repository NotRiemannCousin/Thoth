#pragma once
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <Thoth/Http/NHeaders/Proxy/MultiValueProxy.hpp>
#include <Thoth/Http/NHeaders/Response/Headers/_pch.hpp>

namespace Thoth::Http {
    // based in Microslop's HttpResponseHeaders
    //! @brief Represents the response-specific HTTP header fields.
    //!
    //! The typed accessors use concrete models such as `Cookie`, `Challenge`, `ContentDisposition`, `AcceptRanges` and
    //! `EntityTag`.
    //!
    //! @par Example
    //! @code{.cpp}
    //! ResponseHeaders headers;
    //! headers.Server().Set("thoth");
    //! headers.SetCookie().Add(NHeaders::Cookie{ .name = "session", .value = "abc", .httpOnly = true });
    //! auto server{ headers.Server().Get() };
    //! @endcode
    struct ResponseHeaders : Headers {
        //! @copydoc Headers::Parse
        template<std::ranges::input_range R>
        static std::expected<ResponseHeaders, MessageParseErrorEnum> Parse(R&& headers, size_t maxHeadersLength = 1<<16);

#pragma region Raw and Collection Views

        //! @name Raw and Collection Views
        //! Methods for accessing headers without immediate parsing or as raw collections.
        //! @{

        //! @brief Gets a readonly view of all Set-Cookie values (name/value pairs, no attributes).
        //! @warning Risk of dangling reference if the underlying collection is modified.
        [[nodiscard]] auto GetSetCookiesView() const;

        //! @}
#pragma endregion

#pragma region Response Specific Proxies

        //! @name Response Specific Proxies
        //! @{
        //! Convenient calls to some headers.


#pragma region Multi Value Proxies
        //! @brief All `Set-Cookie` field values present on the response.
        //! @note Each Set-Cookie header line is one `NHeaders::Cookie`.
        //! @par Example
        //! @code{.cpp}
        //! NHeaders::Cookie session{
        //!     .name = "session", .value = "abc", .path = "/", .secure = true };
        //! headers.SetCookie().Set(std::vector<NHeaders::Cookie>{ session });
        //! headers.SetCookie().Add(NHeaders::Cookie{
        //!     .name = "theme", .value = "dark" });
        //! @endcode
        NHeaders::MultiValueProxy<false, NHeaders::Cookie> SetCookie();
        [[nodiscard]] NHeaders::MultiValueProxy<true, NHeaders::Cookie> SetCookie() const;

        //! @brief The authentication challenges offered by the server.
        //! @note May appear multiple times (one challenge per offered scheme).
        //! @par Example
        //! @code{.cpp}
        //! NHeaders::Challenge bearer{
        //!     "Bearer", {{ "realm", "api" }} };
        //! headers.WwwAuthenticate().Add(bearer);
        //! auto realm{ bearer.Param("realm") };
        //! @endcode
        NHeaders::MultiValueProxy<false, NHeaders::Challenge> WwwAuthenticate();
        [[nodiscard]] NHeaders::MultiValueProxy<true, NHeaders::Challenge> WwwAuthenticate() const;

        //! @brief The `Proxy-Authenticate` header.
        //! @note May appear multiple times (one challenge per offered scheme).
        //! @par Example
        //! @code{.cpp}
        //! headers.ProxyAuthenticate().Set(std::vector<NHeaders::Challenge>{
        //!     { "Basic", {{ "realm", "proxy" }} } });
        //! @endcode
        NHeaders::MultiValueProxy<false, NHeaders::Challenge> ProxyAuthenticate();
        [[nodiscard]] NHeaders::MultiValueProxy<true, NHeaders::Challenge> ProxyAuthenticate() const;
#pragma endregion



        //! @brief The `Content-Disposition` header (RFC 6266).
        //! @par Example
        //! @code{.cpp}
        //! auto attachment{ NHeaders::ContentDisposition{
        //!     NHeaders::DispositionTypeHeader{ "attachment" }}
        //!     .WithParam("filename", "report.txt") };
        //! headers.ContentDisposition().Set(attachment);
        //! @endcode
        NHeaders::ValueProxy<false, NHeaders::ContentDisposition> ContentDisposition();
        //! @copybrief ContentDisposition
        [[nodiscard]] NHeaders::ValueProxy<true, NHeaders::ContentDisposition> ContentDisposition() const;


        //! @brief Defines whether the response accepts byte ranges.
        //! @par Example
        //! @code{.cpp}
        //! headers.AcceptRanges().Set(NHeaders::AcceptRanges::Bytes);
        //! auto ranges{ headers.AcceptRanges().GetAsOpt() };
        //! @endcode
        NHeaders::ValueProxy<false, NHeaders::AcceptRanges> AcceptRanges();
        //! @copybrief AcceptRanges
        [[nodiscard]] NHeaders::ValueProxy<true, NHeaders::AcceptRanges> AcceptRanges() const;

        //! @brief Media types accepted for a PATCH request.
        //! @par Example
        //! @code{.cpp}
        //! headers.AcceptPatch().Add(
        //!     NHeaders::MimeTypes::appJson.WithParam("charset", "utf-8"));
        //! @endcode
        NHeaders::ListProxy<false, NHeaders::MimeType> AcceptPatch();
        //! @copybrief AcceptPatch
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::MimeType> AcceptPatch() const;

        //! @brief Media types accepted for a POST request.
        //! @par Example
        //! @code{.cpp}
        //! headers.AcceptPost().Set(std::vector<NHeaders::MimeType>{
        //!     NHeaders::MimeTypes::appJson, NHeaders::MimeTypes::appXml });
        //! @endcode
        NHeaders::ListProxy<false, NHeaders::MimeType> AcceptPost();
        //! @copybrief AcceptPost
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::MimeType> AcceptPost() const;


        //! @brief The apparent age of the response, in seconds.
        //! @par Example
        //! @code{.cpp}
        //! auto age{ headers.Age().GetWithDefault(std::chrono::seconds{ 0 }) };
        //! @endcode
        NHeaders::ValueProxy<false, std::chrono::seconds> Age();
        //! @copybrief Age
        [[nodiscard]] NHeaders::ValueProxy<true, std::chrono::seconds> Age() const;

        // //! @brief Gets or sets the value of the Cache-Control header for an HTTP response.
        // NHeaders::ValueProxy<false, bool> CacheControl();
        // //! @copybrief CacheControl
        // [[nodiscard]] NHeaders::ValueProxy<true, bool> CacheControl() const;

        //! @brief The `ETag` validator of the selected representation.
        //! @par Example
        //! @code{.cpp}
        //! headers.EntityTag().Set(NHeaders::EntityTag{ "0815", true });
        //! @endcode
        NHeaders::ValueProxy<false, NHeaders::EntityTag> EntityTag();
        //! @copybrief EntityTag
        [[nodiscard]] NHeaders::ValueProxy<true, NHeaders::EntityTag> EntityTag() const;

        //! @brief The URL to which this response points.
        //!
        //! @note Can be a relative URL, resolves via @ref Url::Resolve using a proper URL.
        //! @par Example
        //! @code{.cpp}
        //! headers.Location().TrySet("/documents/42");
        //! @endcode
        NHeaders::ValueProxy<false, std::string> Location();
        //! @copybrief Location
        [[nodiscard]] NHeaders::ValueProxy<true, std::string> Location() const;

        //! @brief The date or delay after which the request may be retried.
        //! @par Example
        //! @code{.cpp}
        //! auto retryAfter{ headers.RetryAfter().GetAsOpt() };
        //! @endcode
        NHeaders::ValueProxy<false, std::chrono::utc_clock::time_point, std::chrono::seconds> RetryAfter();
        //! @copybrief RetryAfter
        [[nodiscard]] NHeaders::ValueProxy<true, std::chrono::utc_clock::time_point, std::chrono::seconds> RetryAfter() const;

        //! @brief Identifies the software that generated the response.
        //! @par Example
        //! @code{.cpp}
        //! headers.Server().Set("thoth/1.0");
        //! @endcode
        NHeaders::ValueProxy<false, std::string> Server();
        //! @copybrief Server
        [[nodiscard]] NHeaders::ValueProxy<true, std::string> Server() const;

        // TODO: I dont know if I will implement this
        // //! @brief Gets the value of the Trailer header for an HTTP response.
        // NHeaders::ValueProxy<false, bool> Trailer();
        // //! @copybrief Trailer
        // [[nodiscard]] NHeaders::ValueProxy<true, bool> Trailer() const;


        //! @brief Request headers that select the representation in the response.
        //! @par Example
        //! @code{.cpp}
        //! headers.Vary().Set(std::vector<std::string>{ "Accept", "Accept-Encoding" });
        //! @endcode
        NHeaders::ListProxy<false, std::string> Vary();
        //! @copybrief Vary
        [[nodiscard]] NHeaders::ListProxy<true, std::string> Vary() const;

        //! @}

#pragma endregion
    };
}

#include <Thoth/Http/NHeaders/Response/ResponseHeaders.tpp>
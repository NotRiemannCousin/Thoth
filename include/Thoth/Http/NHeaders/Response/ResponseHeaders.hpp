#pragma once
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <Thoth/Http/NHeaders/Proxy/MultiValueProxy.hpp>
#include <Thoth/Http/NHeaders/Response/Headers/_pch.hpp>

namespace Thoth::Http {
    // based in Microslop's HttpResponseHeaders
    struct ResponseHeaders : Headers {
        //! @copydoc Headers::Parse
        template<std::ranges::input_range R>
        static WebResult<ResponseHeaders> Parse(R&& headers, size_t maxHeadersLength = 1<<16);

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
        //! @brief All Set-Cookie challenges present on the response.
        //! @note Each Set-Cookie header line is one cookie — use @ref MultiValueProxy::Get to read them all.
        NHeaders::MultiValueProxy<false, NHeaders::Cookie> SetCookie();
        [[nodiscard]] NHeaders::MultiValueProxy<true, NHeaders::Cookie> SetCookie() const;

        //! @brief The types of authentication that the server utilizes.
        //! @note May appear multiple times (one challenge per offered scheme).
        NHeaders::MultiValueProxy<false, NHeaders::Challenge> WwwAuthenticate();
        [[nodiscard]] NHeaders::MultiValueProxy<true, NHeaders::Challenge> WwwAuthenticate() const;

        //! @brief The "proxy-authenticate" header.
        //! @note May appear multiple times (one challenge per offered scheme).
        NHeaders::MultiValueProxy<false, NHeaders::Challenge> ProxyAuthenticate();
        [[nodiscard]] NHeaders::MultiValueProxy<true, NHeaders::Challenge> ProxyAuthenticate() const;
#pragma endregion



        //! @brief The Content-Disposition header (RFC 6266) - "attachment" vs "inline", with params like filename.
        NHeaders::ValueProxy<false, NHeaders::ContentDisposition> ContentDisposition();
        //! @copybrief ContentDisposition
        [[nodiscard]] NHeaders::ValueProxy<true, NHeaders::ContentDisposition> ContentDisposition() const;


        //! @brief Defines if the response accepts bytes or no (just "bytes" or "none" is available).
        NHeaders::ValueProxy<false, NHeaders::AcceptRanges> AcceptRanges();
        //! @copybrief AcceptRanges
        [[nodiscard]] NHeaders::ValueProxy<true, NHeaders::AcceptRanges> AcceptRanges() const;

        //! @brief The value of the Accept-Patch header.
        NHeaders::ListProxy<false, NHeaders::MimeType> AcceptPatch();
        //! @copybrief AcceptPatch
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::MimeType> AcceptPatch() const;

        //! @brief The value of the Accept-Patch header.
        NHeaders::ListProxy<false, NHeaders::MimeType> AcceptPost();
        //! @copybrief AcceptPost
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::MimeType> AcceptPost() const;


        //! @brief Get how old this response has generated in the server, useful for caching.
        NHeaders::ValueProxy<false, std::chrono::seconds> Age();
        //! @copybrief Age
        [[nodiscard]] NHeaders::ValueProxy<true, std::chrono::seconds> Age() const;

        // //! @brief Gets or sets the value of the Cache-Control header for an HTTP response.
        // NHeaders::ValueProxy<false, bool> CacheControl();
        // //! @copybrief CacheControl
        // [[nodiscard]] NHeaders::ValueProxy<true, bool> CacheControl() const;

        //! @brief The "ETag" header.
        NHeaders::ValueProxy<false, NHeaders::EntityTag> EntityTag();
        //! @copybrief EntityTag
        [[nodiscard]] NHeaders::ValueProxy<true, NHeaders::EntityTag> EntityTag() const;

        //! @brief The URL where this response pointers to.
        //!
        //! @note Can be a relative URL, resolves via @ref Url::Resolve using a proper URL.
        NHeaders::ValueProxy<false, std::string> Location();
        //! @copybrief Location
        [[nodiscard]] NHeaders::ValueProxy<true, std::string> Location() const;

        //! @brief The date or cooldown when the endpoint will accept new responses.
        NHeaders::ValueProxy<false, std::chrono::utc_clock::time_point, std::chrono::seconds> RetryAfter();
        //! @copybrief RetryAfter
        [[nodiscard]] NHeaders::ValueProxy<true, std::chrono::utc_clock::time_point, std::chrono::seconds> RetryAfter() const;

        //! @brief The type of server.
        NHeaders::ValueProxy<false, std::string> Server();
        //! @copybrief Server
        [[nodiscard]] NHeaders::ValueProxy<true, std::string> Server() const;

        // TODO: I dont know if I will implement this
        // //! @brief Gets the value of the Trailer header for an HTTP response.
        // NHeaders::ValueProxy<false, bool> Trailer();
        // //! @copybrief Trailer
        // [[nodiscard]] NHeaders::ValueProxy<true, bool> Trailer() const;


        //! @brief The "vary" header.
        NHeaders::ListProxy<false, std::string> Vary();
        //! @copybrief Vary
        [[nodiscard]] NHeaders::ListProxy<true, std::string> Vary() const;

        //! @}

#pragma endregion
    };
}

#include <Thoth/Http/NHeaders/Response/ResponseHeaders.tpp>
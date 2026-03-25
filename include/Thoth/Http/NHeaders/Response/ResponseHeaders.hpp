#pragma once
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <Thoth/Http/NHeaders/Response/Headers/_pch.hpp>

namespace Thoth::Http {
    // based in Microslop's HttpResponseHeaders
    struct ResponseHeaders : Headers {

#pragma region Response Specific Proxies

        //! @name Response Specific Proxies
        //! @{
        //! Convenient calls to some headers.


        //! @brief Defines if the response accepts bytes or no (just "bytes" or "none" is available).
        NHeaders::ValueProxy<false, NHeaders::AcceptRanges> AcceptRanges();
        //! @copybrief AcceptRanges
        [[nodiscard]] NHeaders::ValueProxy<true, NHeaders::AcceptRanges> AcceptRanges() const;

        //! @brief The value of the Accept-Patch header.
        NHeaders::ListProxy<false, NHeaders::MimeType> AcceptPatch();
        //! @copybrief AcceptPatch
        NHeaders::ListProxy<true, NHeaders::MimeType> AcceptPatch() const;

        //! @brief The value of the Accept-Patch header.
        NHeaders::ListProxy<false, NHeaders::MimeType> AcceptPost();
        //! @copybrief AcceptPost
        NHeaders::ListProxy<true, NHeaders::MimeType> AcceptPost() const;


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
        NHeaders::ValueProxy<false, std::string> Location(); // TODO: Improve Url before change the type
        //! @copybrief Location
        [[nodiscard]] NHeaders::ValueProxy<true, std::string> Location() const;

        // //! @brief Gets a view of the contents of this headers collection that does not parse nor validate the data upon access.
        // NHeaders::ValueProxy<false, bool> NonValidated();
        // //! @copybrief NonValidated
        // [[nodiscard]] NHeaders::ValueProxy<true, bool> NonValidated() const;

        //! @brief The "proxy-authenticate" header.
        NHeaders::ValueProxy<false, std::string> ProxyAuthenticate();
        //! @copybrief ProxyAuthenticate
        [[nodiscard]] NHeaders::ValueProxy<true, std::string> ProxyAuthenticate() const;

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


        //! @brief The types of authentication that the server utilizes.
        NHeaders::ValueProxy<false, std::string> WwwAuthenticate();
        //! @copybrief WwwAuthenticate
        [[nodiscard]] NHeaders::ValueProxy<true, std::string> WwwAuthenticate() const;

        //! @}

#pragma endregion
    };
}

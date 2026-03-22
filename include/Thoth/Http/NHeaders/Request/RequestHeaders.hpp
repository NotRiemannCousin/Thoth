#pragma once
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <vector>

namespace Thoth::Http::NHeaders {
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
        [[nodiscard]] auto GetNonValidatedView() const;

        //! @brief Gets a readonly view of all Cookie values.
        //! @warning Risk of dangling reference if the underlying collection is modified.
        [[nodiscard]] auto GetCookiesView() const;

        //! @}
#pragma endregion

#pragma region Strongly Typed Headers (Getters/Setters)
        //! @name Strongly Typed Headers (Getters/Setters)
        //! Headers that usually contain a single value or a specific structured format.
        //! @{

        // TODO: Implement (someday)    
        // ! @brief Gets the Authorization header (e.g., Bearer, Basic).
        // void GetAuthorization();
        // ! @brief Sets the Authorization header (e.g., Bearer, Basic).
        // void SetAuthorization();

        //! @brief Gets the Host header. Essential for HTTP/1.1 and SNI.
        ExpectedHeader<Url> GetHost();
        //! @brief Sets the Host header. Essential for HTTP/1.1 and SNI.
        void SetHost(string host);
        //! @brief Sets the Host header. Essential for HTTP/1.1 and SNI.
        void SetHost(Url host);

        //! @brief Gets or sets the Referer header (yes, it's misspelled in RFC).
        ExpectedHeader<Url> GetReferrer();
        void SetReferrer(string url);
        void SetReferrer(Url url);

        //! @brief Gets or sets the Origin header. Used for CORS requests.
        ExpectedHeader<std::variant<Url, std::monostate>> GetOrigin();
        void SetOrigin(string origin);
        void SetOrigin(Url origin);
        void SetOrigin(std::monostate);

        //! @brief Gets or sets the From header (email address of the user).
        string GetFrom();
        void SetFrom(string email);

        //! @brief Gets or sets the Max-Forwards header for limiting proxy hops.
        void GetMaxForwards();
        void SetMaxForwards();

        //! @brief Gets or sets the :protocol pseudo-header (primarily for HTTP/2/3).
        void GetProtocol();
        void SetProtocol();

        //! @brief Gets or sets the Proxy-Authorization header.
        void GetProxyAuthorization();
        void SetProxyAuthorization();

        //! @brief Gets or sets the Range header for partial content requests.
        void GetRange();
        void SetRange();

        //! @brief Gets or sets the Cache-Control header for the request.
        void GetCacheControl();
        void SetCacheControl();

        //! @brief Gets or sets the Date header.
        void GetDate();
        void SetDate();
        //! @}
#pragma endregion

#pragma region Conditional Headers
        //! @name Conditional Headers
        //! Headers used for cache validation and conditional requests.
        //! @{

        //! @brief Gets or sets the If-Modified-Since header.
        void GetIfModifiedSince();
        void SetIfModifiedSince();

        //! @brief Gets or sets the If-Unmodified-Since header.
        void GetIfUnmodifiedSince();
        void SetIfUnmodifiedSince();

        //! @brief Gets or sets the If-Range header.
        void GetIfRange();
        void SetIfRange();

        //! @brief Gets or sets the If-Match header.
        void GetIfMatch();
        void SetIfMatch();

        //! @brief Gets or sets the If-None-Match header.
        void GetIfNoneMatch();
        void SetIfNoneMatch();
        //! @}
#pragma endregion

#pragma region Multi-Value Headers (Adders/Modifiers)
        //! @name Multi-Value Headers (Adders/Modifiers)
        //! Headers that naturally support multiple comma-separated values.
        //! @{

        //! @brief Adds a value to the Accept header (media types).
        void AddAccept(MimeType type);

        //! @brief Adds a value to the Accept-Charset header.
        void AddAcceptCharset();

        //! @brief Adds a value to the Accept-Encoding header (gzip, br, etc).
        void AddAcceptEncoding();

        //! @brief Adds a value to the Accept-Language header.
        void AddAcceptLanguage();

        //! @brief Adds a value to the TE (Transfer Encoding) header.
        void AddTe();

        //! @brief Adds a value to the Upgrade header (e.g., websocket).
        void AddUpgrade();

        //! @brief Adds a value to the Via header for proxy tracking.
        void AddVia();
        //! @}
#pragma endregion

#pragma region CORS Request Headers
        //! @name CORS Request Headers
        //! Headers sent by the client during Cross-Origin Resource Sharing.
        //! @{

        //! @brief Gets or sets the Access-Control-Request-Method for preflight requests.
        void GetAccessControlRequestMethod();
        void SetAccessControlRequestMethod();

        //! @brief Gets or sets the Access-Control-Request-Headers for preflight requests.
        void GetAccessControlRequestHeaders();
        void SetAccessControlRequestHeaders();
        //! @}
#pragma endregion

#pragma region Convenience Methods
        //! @name Convenience Methods
        //! Boolean checks and simplified abstractions over complex headers.
        //! @{

        //! @brief Checks if the Connection header contains the 'close' token.
        [[nodiscard]] bool IsConnectionClose() const;

        //! @brief Checks if the Expect header specifically contains '100-continue'.
        [[nodiscard]] bool IsExpectContinue() const;

        //! @brief Checks if the Transfer-Encoding header contains 'chunked'.
        [[nodiscard]] bool IsTransferEncodingChunked() const;

        //! @brief Sets the Transfer-Encoding header to 'chunked' or removes it.
        void SetTransferEncodingChunked(bool chunked);

        //! @brief Sets the Connection header to 'Close' or 'Keep-Alive'.
        void SetConnectionClose(bool close);
        //! @}
#pragma endregion
    };
};
#pragma once
#include <Thoth/Http/NHeaders/Headers.hpp>

namespace Thoth::Http::NHeaders {
    // based in Microslop's HttpResponseHeaders
    struct ResponseHeaders : Headers {
        // AcceptRanges
        // Gets the value of the Accept-Ranges header for an HTTP response.
        //
        // Age
        // Gets or sets the value of the Age header for an HTTP response.
        //
        // CacheControl
        // Gets or sets the value of the Cache-Control header for an HTTP response.
        //
        // Connection
        // Gets the value of the Connection header for an HTTP response.
        //
        // ConnectionClose
        // Gets or sets a value that indicates if the Connection header for an HTTP response contains Close.
        //
        // Date
        // Gets or sets the value of the Date header for an HTTP response.
        //
        // ETag
        // Gets or sets the value of the ETag header for an HTTP response.
        //
        // Location
        // Gets or sets the value of the Location header for an HTTP response.
        //
        // NonValidated
        // Gets a view of the contents of this headers collection that does not parse nor validate the data upon access.
        //
        // (Inherited from HttpHeaders)
        // Pragma
        // Gets the value of the Pragma header for an HTTP response.
        //
        // ProxyAuthenticate
        // Gets the value of the Proxy-Authenticate header for an HTTP response.
        //
        // RetryAfter
        // Gets or sets the value of the Retry-After header for an HTTP response.
        //
        // Server
        // Gets the value of the Server header for an HTTP response.
        //
        // Trailer
        // Gets the value of the Trailer header for an HTTP response.
        //
        // TransferEncoding
        // Gets the value of the Transfer-Encoding header for an HTTP response.
        //
        // TransferEncodingChunked
        // Gets or sets a value that indicates if the Transfer-Encoding header for an HTTP response contains chunked.
        //
        // Upgrade
        // Gets the value of the Upgrade header for an HTTP response.
        //
        // Vary
        // Gets the value of the Vary header for an HTTP response.
        //
        // Via
        // Gets the value of the Via header for an HTTP response.
        //
        // Warning
        // Gets the value of the Warning header for an HTTP response.
        //
        // WwwAuthenticate
        // Gets the value of the WWW-Authenticate header for an HTTP response.Accept 
    };
}

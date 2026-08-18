#pragma once
#include <Hermes/Endpoint/IpEndpoint/IpAddress.hpp>

namespace Thoth::Http {
        template<MethodConcept Method, WritableBodyConcept ResponseBody>
    struct Response;


    template<class F, class Body>
    concept ResponseBodyFactoryConcept = BodyFactoryConcept<F, Body, ResponseHead>;




    //! @brief Configuration options for HTTP exchanges performed by @ref Client.
    //!
    //! Options related to connection establishment (@ref connectionTimeout,
    //! @ref handshakeTimeout, @ref ignoreCertificateErrors, @ref requestMutualAuth)
    //! are applied only when a new connection is created. When a pooled connection
    //! is reused, these fields are silently ignored.
    struct ClientOptions {

        //! @brief Maximum duration allowed for a complete HTTP exchange (send + receive).
        //!
        //! Measured from the moment the request is handed to the socket until the
        //! last byte of the response body is received.
        //!
        std::chrono::milliseconds requestTimeout{ std::chrono::milliseconds::max() };

        //! @brief Maximum duration allowed for establishing a TCP connection.
        //!
        //! Ignored when a pooled connection is reused.
        std::chrono::milliseconds connectionTimeout{ std::chrono::seconds{ 300 } };

        //! @brief Maximum duration allowed for the TLS handshake.
        //!
        //! Has no effect on plain HTTP connections.
        //! Ignored when a pooled connection is reused.
        std::chrono::milliseconds handshakeTimeout{ std::chrono::seconds{ 300 } };

        //! @brief If `true`, TLS certificate validation errors are suppressed.
        //!
        //! @warning Disabling certificate validation exposes the connection to
        //! man-in-the-middle attacks. Use only in controlled environments.
        //!
        //! Has no effect on plain HTTP connections.
        //! Ignored when a pooled connection is reused.
        bool ignoreCertificateErrors{};

        //! @brief If `true`, mutual TLS authentication is requested during the handshake.
        //!
        //! Requires the peer to present a valid certificate in addition to the server's own.
        //!
        //! Has no effect on plain HTTP connections.
        //! Ignored when a pooled connection is reused.
        bool requestMutualAuth{};
    };


    //! @brief Stores per-connection information for an open socket managed by a Thoth HTTP client.
    //! @details
    //! Aggregates the low-level socket (Hermes::RawTlsClient), the HTTP version
    //! negotiated for the connection, and the timestamp of the last use.
    //! This struct is used internally by ClientJanitor to decide when a socket can
    //! be safely evicted.
    struct ClientConnection {
        // TODO: FUTURE: Implement HTTP2 and Quic
        using SocketType = std::variant<Hermes::RawTcpClient, Hermes::RawTlsClient>;
        using Deadline   = Hermes::DefaultTransferPolicy<>::Deadline;

        struct SendOptions { std::optional<Deadline> deadline{}; };

        //! The Hermes TLS client socket. Ensure that any operation on this
        //! socket respects thread-safety constraints; ClientJanitor may access sockets
        //! from a background thread.
        SocketType socket;
        //! The HTTP version of the connection (e.g., HTTP/1.1). Future HTTP/2
        //! or QUIC support will extend this field.
        VersionEnum version;

        //! A timestamp marking the most recent use of this socket. ClientJanitor uses this to detect
        //! idle connections.
        std::chrono::steady_clock::time_point lastUsed;


        template<class T>
        auto Send(const T& data);

        template<class T>
        auto Send(const T& data, SendOptions options);

        void Close();
        void Abort();
    };

    struct ClientConnectionKey {
        Hermes::IpEndpoint endpoint;
        std::string scheme;
        std::string hostname;

        bool operator==(const ClientConnectionKey& other) const = default;
    };
}

#include <Thoth/Http/Client/Definitions.tpp>

namespace Thoth::Http {
    static_assert(ConnectionConcept<ClientConnection>);
}
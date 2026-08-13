#pragma once
// TODO: #include <Hermes/Socket/Async/AsyncClientSocket.hpp>
#include <Hermes/Socket/Sync/ClientSocket.hpp>
#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/ExchangeError.hpp>

#include <future>
#include <chrono>
#include <atomic>
#include <mutex>

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
        //! @note Not yet enforced. Implementation is pending changes to @ref Hermes::DefaultSocketData.
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
        void Close();
        void Abort();
    };


    //! @brief Manages a pool of reusable HTTP sockets to optimize consecutive calls.
    //! @details
    //! ClientJanitor maintains a map from endpoint identifiers to vectors of live
    //! sockets, allowing the Client to reuse connections instead of repeatedly
    //! establishing new ones. A background janitor thread periodically sweeps sockets
    //! that have been idle for more than a configurable threshold (currently 1 minute).
    //!
    //! **Thread-safety:**
    //! - The internal `connectionPool` must be accessed while holding `poolMutex`.
    //! - Do not assume any locking is performed automatically; always lock `poolMutex`
    //!   before reading or writing `connectionPool`.
    //! - The janitor thread runs every ~30 seconds and will also lock `poolMutex` during
    //!   its sweep.
    //!
    //! **Lifetime:**
    //! - ClientJanitor is a singleton; access it via `Instance()`.
    //! - The background thread is stopped when the singleton is destroyed.
    //!
    //! **Use-cases:**
    //! - Advanced users can inspect `connectionPool` for statistics or debugging.
    //! - Most users should rely on the Client APIs directly and ignore this class.
    struct ClientJanitor {

        static ClientJanitor& Instance();
        void JanitorLoop(std::stop_token stopToken);

        std::mutex poolMutex;

        //! @brief Group multiple sockets connected to the same endpoint. Before using it lock the poolMutex
        //! to not break other threads.
        std::unordered_map<Hermes::IpEndpoint, std::vector<std::shared_ptr<ClientConnection>>> connectionPool;
    private:
        ClientJanitor();

        std::jthread m_janitorThread;
    };


    //! @brief Class that transforms requests with a given method AndParse their responses,
    //! monad friendly.
    //!
    //! Supports sync and async operations (only sync are implemented at the given moment).

    struct Client {
        //! @brief Alias for `std::expected` containing a Response or a ExchangeError.
        //!
        //! This type is used consistently across the Client API to represent the result
        //! of a synchronous HTTP operation. It follows the modern C++ "error handling
        //! as value" pattern.
        //!
        //! @tparam Method The HTTP method used for the request and present in the response.
        //! @tparam Body The body type of the response (e.g., String, Json, etc.).
        template<class Method,  WritableBodyConcept Body>
        using ExpResponse = std::expected<Response<Method, Body>, ExchangeError>;

        using SocketPtr = std::shared_ptr<ClientConnection>;

        //! @brief Sends a synchronous HTTP request and receives a response of the same body type.
        //! @details
        //! This is the primary entry point for simple request-response cycles where the
        //! structure of the data sent (RequestBody) is expected to be identical to the
        //! structure of the data received (ResponseBody).
        //!
        //! **Type Relationship:**
        //! - The Request uses `Body` as its payload.
        //! - The Response uses `Body` as its payload.
        //!
        //! **Input Requirements:**
        //! - `Body` must satisfy be `std::default_initializable`. The library must be able
        //!   to construct the response body (`Body{}`) before reading the network stream.
        //!   If your body type requires context from the HTTP headers to be constructed
        //!   (e.g., a parser that needs to know the `Content-Length` or `Content-Type`
        //!   upfront), use `SendAndParse` or `SendAsAndParse` instead.
        //!
        //! @tparam Method The HTTP method type (e.g., `Methods::Get`).
        //! @tparam Body The body type for both request and response. Must be default-initializable.
        //! @param request The fully configured HTTP request to send.
        //! @param opts Connection options.
        //! @return `ExpResponse<Method, Body>` containing the populated response or an error.
        template<MethodConcept Method, BodyConcept Body>
            requires std::default_initializable<Body>
        static auto Send(Request<Method, Body> request, ClientOptions opts = {}) -> ExpResponse<Method, Body>;

        //! @brief Sends synced (thread blocking) requests.
        template<MethodConcept Method, BodyConcept Body, class F>
            requires ResponseBodyFactoryConcept<F, Body>
        static auto SendAndParse(Request<Method, Body> request, F&& bodyFactory, ClientOptions opts = {}) -> ExpResponse<Method, Body>;

        //! @brief Sends synced (thread blocking) requests.
        template<MethodConcept Method, ReadableBodyConcept RequestBody, WritableBodyConcept ResponseBody>
            requires std::default_initializable<ResponseBody>
        static auto SendAs(Request<Method, RequestBody> request, ClientOptions opts = {}) -> ExpResponse<Method, ResponseBody>;

        //! @brief Sends a request with one body type and receives a response of another, constructed via factory.
        //! @details
        //! The most flexible synchronous send method. It combines the type decoupling of
        //! `SendAs` with the factory-based initialization of `SendAndParse`.
        //!
        //! **Factory Contract:**
        //! - Similar to `SendAndParse`, the `bodyFactory` is called with `ResponseHead`.
        //! - It must return `std::expected<ResponseBody, ExchangeError>`.
        //!
        //! **Use Cases:**
        //! - Uploading a file (`RequestBody` = `File`) and downloading a processing report (`ResponseBody` = `Json`),
        //!   where the JSON parser needs configuration based on headers.
        //!
        //! **Input Requirements:**
        //! - `ResponseBody`: Must satisfy `WritableBodyConcept`.
        //! - `F`: Must satisfy `ResponseBodyFactoryConcept<F, ResponseBody>`.
        //!
        //! @tparam Method The HTTP method type.
        //! @tparam RequestBody The type of the body being sent.
        //! @tparam ResponseBody The type of the body expected in the response.
        //! @tparam F The factory type (callable).
        //! @param request The HTTP request to send.
        //! @param bodyFactory A callable `F(const ResponseHead&)` returning `std::expected<ResponseBody, ExchangeError>`.
        //! @param opts Connection Options.
        //! @return `ExpResponse<Method, ResponseBody>` containing the response.
        template<MethodConcept Method, ReadableBodyConcept RequestBody, WritableBodyConcept ResponseBody, class F>
            requires ResponseBodyFactoryConcept<F, ResponseBody>
        static auto SendAsAndParse(Request<Method, RequestBody> request, F&& bodyFactory, ClientOptions opts = {}) -> ExpResponse<Method, ResponseBody>;

        //! @hof{Send}
        static constexpr auto H_Send(ClientOptions opts = {});

        template<class F>
        static auto H_SendAndParse(F&& bodyFactory, ClientOptions opts = {});

        //! @hof{SendAs}
        template<WritableBodyConcept ResponseBody>
        static constexpr auto H_SendAs(ClientOptions opts = {});

        //! @hof{SendAsAndParse}
        template<WritableBodyConcept ResponseBody, class F>
            requires ResponseBodyFactoryConcept<F, ResponseBody>
        static auto H_SendAsAndParse(F&& bodyFactory, ClientOptions opts = {});

    private:
        // I will do it when... Idk
        // template<MethodConcept Method, WritableBodyConcept ResponseBody>
        // static std::expected<std::pair<SocketPtr, Response<Method, ResponseBody>>, ExchangeError> 1RawSend_(Request<Method> request);

        template<MethodConcept Method, WritableBodyConcept ResponseBody, class F>
            requires ResponseBodyFactoryConcept<F, ResponseBody>
        static std::expected<std::pair<SocketPtr, Response<Method, ResponseBody>>, ExchangeError> ParseHttp1_(SocketPtr infoPtr, F&& bodyFactory);

        // template<MethodConcept Method, WritableBodyConcept ResponseBody>
        // static std::expected<std::pair<SocketPtr, Response<Method, ResponseBody>>, ExchangeError> request();
        //
        // template<MethodConcept Method>
        // static std::expected<Response<Method>, ExchangeError> 3DlsSend_(Request<Method> request);
        // friend ClientJanitor;
    };
}

#include <Thoth/Http/Client.tpp>


namespace Thoth::Http {
    static_assert(ConnectionConcept<ClientConnection>);
}
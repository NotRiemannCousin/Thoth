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


    //! @brief Stores per-connection information for an open socket managed by a Thoth HTTP client.
    //! @details
    //! Aggregates the low-level socket (Hermes::RawTlsClient), the HTTP version
    //! negotiated for the connection, and the timestamp of the last use.
    //! This struct is used internally by ClientJanitor to decide when a socket can
    //! be safely evicted.
    struct ClientConnection {
        // TODO: FUTURE: Implement HTTP2 and Quic
        // using ClientSocketType = std::variant<Hermes::RawTcpClient     , Hermes::RawTlsClient      ,
        //                                       Hermes::AsyncRawTcpClient, Hermes::AsyncRawTlsClient>;
        //! The Hermes TLS client socket. Ensure that any operation on this
        //! socket respects thread-safety constraints; ClientJanitor may access sockets
        //! from a background thread.
        Hermes::RawTlsClient socket;
        //! The HTTP version of the connection (e.g., HTTP/1.1). Future HTTP/2
        //! or QUIC support will extend this field.
        VersionEnum version;

        //! A timestamp marking the most recent use of this socket. ClientJanitor uses this to detect
        //! idle connections.
        std::chrono::steady_clock::time_point lastUsed;
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
        void JanitorLoop();

        std::mutex poolMutex;

        //! @brief Group multiple sockets connected to the same endpoint. Before using it lock the poolMutex
        //! to not break other threads.
        std::unordered_map<Hermes::IpEndpoint, std::vector<std::shared_ptr<ClientConnection>>> connectionPool;
    private:
        std::atomic_bool _isRunning{ true };
        ClientJanitor();
        ~ClientJanitor();

        std::jthread _janitorThread;
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
        template<class Method, BodyConcept Body>
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
        //! @return `ExpResponse<Method, Body>` containing the populated response or an error.
        template<MethodConcept Method, BodyConcept Body>
            requires std::default_initializable<Body>
        static auto Send(Request<Method, Body> request) -> ExpResponse<Method, Body>;

        //! @brief Sends synced (thread blocking) requests.
        template<MethodConcept Method, BodyConcept Body, class F>
            requires ResponseBodyFactoryConcept<F, Body>
        static auto SendAndParse(Request<Method, Body> request, F&& bodyFactory) -> ExpResponse<Method, Body>;

        //! @brief Sends synced (thread blocking) requests.
        template<MethodConcept Method, ReadableBodyConcept RequestBody, WritableBodyConcept ResponseBody>
            requires std::default_initializable<ResponseBody>
        static auto SendAs(Request<Method, RequestBody> request) -> ExpResponse<Method, ResponseBody>;

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
        //! @return `ExpResponse<Method, ResponseBody>` containing the response.
        template<MethodConcept Method, ReadableBodyConcept RequestBody, WritableBodyConcept ResponseBody, class F>
            requires ResponseBodyFactoryConcept<F, ResponseBody>
        static auto SendAsAndParse(Request<Method, RequestBody> request, F&& bodyFactory) -> ExpResponse<Method, ResponseBody>;

        //! @hof{Send}
        static constexpr auto H_Send();

        template<class F>
        static auto H_SendAndParse(F&& bodyFactory);

        //! @hof{SendAs}
        template<WritableBodyConcept ResponseBody>
        static constexpr auto H_SendAs();

        //! @hof{SendAsAndParse}
        template<WritableBodyConcept ResponseBody, class F>
            requires ResponseBodyFactoryConcept<F, ResponseBody>
        static auto H_SendAsAndParse(F&& bodyFactory);

    private:
        // I will do it when... Idk
        // template<MethodConcept Method, WritableBodyConcept ResponseBody>
        // static std::expected<std::pair<SocketPtr, Response<Method, ResponseBody>>, ExchangeError> 1RawSend_(Request<Method> request);

        template<MethodConcept Method, WritableBodyConcept ResponseBody, class F>
            requires ResponseBodyFactoryConcept<F, ResponseBody>
        static std::expected<std::pair<SocketPtr, Response<Method, ResponseBody>>, ExchangeError> ParseHttp11_(SocketPtr infoPtr, F&& bodyFactory);

        // template<MethodConcept Method, WritableBodyConcept ResponseBody>
        // static std::expected<std::pair<SocketPtr, Response<Method, ResponseBody>>, ExchangeError> request();
        //
        // template<MethodConcept Method>
        // static std::expected<Response<Method>, ExchangeError> 3DlsSend_(Request<Method> request);
        // friend ClientJanitor;
    };
}

#include <Thoth/Http/Client.tpp>
#pragma once
// TODO: #include <Hermes/Socket/Async/AsyncClientSocket.hpp>
#include <Hermes/Socket/Sync/ClientSocket.hpp>
#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/Client/Definitions.hpp>
#include <Thoth/ThothError.hpp>

#include <future>
#include <chrono>
#include <optional>


namespace Thoth::Http {
    struct ClientConnection;
    //! @brief Class that transforms requests with a given method AndParse their responses,
    //! monad friendly.
    //!
    //! Supports sync and async operations (only sync are implemented at the given moment).

    struct Client {
        //! @brief Alias for `std::expected` containing a Response or a ThothError.
        //!
        //! This type is used consistently across the Client API to represent the result
        //! of a synchronous HTTP operation. It follows the modern C++ "error handling
        //! as value" pattern.
        //!
        //! @tparam Method The HTTP method used for the request and present in the response.
        //! @tparam Body The body type of the response (e.g., String, Json, etc.).
        template<class Method,  WritableBodyConcept Body>
        using ExpResponse = std::expected<Response<Method, Body>, ThothError>;

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
        //! - It must return `std::expected<ResponseBody, ThothError>`.
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
        //! @param bodyFactory A callable `F(const ResponseHead&)` returning `std::expected<ResponseBody, ThothError>`.
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
        // static std::expected<std::pair<SocketPtr, Response<Method, ResponseBody>>, ThothError> 1RawSend_(Request<Method> request);

        template<MethodConcept Method, WritableBodyConcept ResponseBody, class F>
            requires ResponseBodyFactoryConcept<F, ResponseBody>
        static std::expected<std::pair<SocketPtr, Response<Method, ResponseBody>>, ThothError> ParseHttp1_(
            SocketPtr infoPtr, F&& bodyFactory, std::optional<ClientConnection::Deadline> deadline,
            std::size_t maxBodyLength);

        // template<MethodConcept Method, WritableBodyConcept ResponseBody>
        // static std::expected<std::pair<SocketPtr, Response<Method, ResponseBody>>, ThothError> request();
        //
        // template<MethodConcept Method>
        // static std::expected<Response<Method>, ThothError> 3DlsSend_(Request<Method> request);
        // friend ClientJanitor;
    };
}

#include <Thoth/Http/Client/Client.tpp>
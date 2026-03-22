#pragma once
#include <Hermes/Socket/ClientSocket.hpp>
#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/RequestError.hpp>

#include <future>
#include <chrono>
#include <atomic>
#include <mutex>

namespace Thoth::Http {
    template<MethodConcept Method, ResponseBodyConcept ResponseBody>
    struct Response;

    //! @brief structure that stores info about an open Socket, like the version,
    //! the type of socket and the last used time.
    struct Socket {
        // TODO: FUTURE: Implement HTTP2 and Quic
        // using ClientSocketType = std::variant<Hermes::RawTcpClient, Hermes::RawDtlsClient>;
        Hermes::RawTlsClient socket;
        VersionEnum version;

        std::chrono::steady_clock::time_point lastUsed;
    };


    //! @brief The ClientJanitor stores all open Sockets in a pool to optimize consecutive calls.
    //! Only use it if you know what you are doing.
    //!
    //! Please use poolMutex while accessing connectionPool to not break other threads.
    //! Each 30s it will destruct sockets unused for more than 1 minute.
    struct ClientJanitor {

        static ClientJanitor& Instance();
        void JanitorLoop();

        std::mutex poolMutex;

        //! @brief Group multiple sockets connected to the same endpoint. Before using it lock the poolMutex
        //! to not break other threads.
        std::unordered_map<Hermes::IpEndpoint, std::vector<std::shared_ptr<Socket>>> connectionPool;
    private:
        std::atomic_bool _isRunning{ true };
        ClientJanitor();
        ~ClientJanitor();

        std::jthread _janitorThread;
    };


    //! @brief Class that transforms requests with a given method into their responses,
    //! monad friendly.
    //!
    //! Supports sync and async operations (only sync are implemented at the given moment).

    template<class T>
    concept BodyConcept = RequestBodyConcept<T> && ResponseBodyConcept<T>;

    template<class F, class Body>
    concept ResponseBodyFactoryConcept = ResponseBodyConcept<Body> &&
            requires (F f, const ResponseHead& head){ { std::invoke(f, head) } -> std::same_as<std::expected<Body, RequestError>>; };

    struct Client {
        //! @brief Sends synced (thread blocking) requests.
        template<MethodConcept Method, BodyConcept Body>
            requires std::default_initializable<Body>
        static expected<Response<Method, Body>, RequestError> Send(Request<Method, Body> request);

        //! @brief Sends synced (thread blocking) requests.
        template<MethodConcept Method, BodyConcept Body, class F>
            requires ResponseBodyFactoryConcept<F, Body>
        static expected<Response<Method, Body>, RequestError> SendAndRecvInto(Request<Method, Body> request, F&& bodyFactory);

        //! @brief Sends synced (thread blocking) requests.
        template<MethodConcept Method, RequestBodyConcept RequestBody, ResponseBodyConcept ResponseBody>
            requires std::default_initializable<ResponseBody>
        static expected<Response<Method, ResponseBody>, RequestError> SendAndRecvAs(Request<Method, RequestBody> request);

        //! @brief Sends synced (thread blocking) requests.
        template<MethodConcept Method, RequestBodyConcept RequestBody, ResponseBodyConcept ResponseBody, class F>
            requires ResponseBodyFactoryConcept<F, ResponseBody>
        static expected<Response<Method, ResponseBody>, RequestError> SendAndRecvAsInto(Request<Method, RequestBody> request, F&& bodyFactory);

        //! @hof{Send}
        static constexpr auto H_Send();

        template<class F>
        static auto H_SendAndRecvInto(F&& bodyFactory);

        //! @hof{SendAndGetAs}
        template<ResponseBodyConcept ResponseBody>
        static constexpr auto H_SendAndRecvAs();

        template<ResponseBodyConcept ResponseBody, class F>
            requires ResponseBodyFactoryConcept<F, ResponseBody>
        static auto H_SendAndRecvAsInto(F&& bodyFactory);

        using SocketPtr = std::shared_ptr<Socket>;

    private:
        // I will do it when... Idk
        // template<MethodConcept Method, ResponseBodyConcept ResponseBody>
        // static expected<std::pair<SocketPtr, Response<Method, ResponseBody>>, RequestError> _1RawSend(Request<Method> request);

        template<MethodConcept Method, ResponseBodyConcept ResponseBody, class F>
            requires ResponseBodyFactoryConcept<F, ResponseBody>
        static expected<std::pair<SocketPtr, Response<Method, ResponseBody>>, RequestError> P_ParseHttp11(SocketPtr infoPtr, F&& bodyFactory);

        // template<MethodConcept Method, ResponseBodyConcept ResponseBody>
        // static expected<std::pair<SocketPtr, Response<Method, ResponseBody>>, RequestError> request();
        //
        // template<MethodConcept Method>
        // static expected<Response<Method>, RequestError> _3DlsSend(Request<Method> request);
        // friend ClientJanitor;
    };
}

#include <Thoth/Http/Client.tpp>
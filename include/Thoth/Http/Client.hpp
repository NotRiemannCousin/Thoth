#pragma once
#include <Hermes/Socket/ClientSocket.hpp>
#include <Thoth/Http/Request/Request.hpp>

#include <future>
#include <chrono>
#include <atomic>
#include <mutex>

namespace Thoth::Http {
    template<MethodConcept Method>
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
    struct Client {
        //! @brief Sends synced (thread blocking) requests.
        template<MethodConcept Method = GetMethod>
        static expected<Response<Method>, string> Send(Request<Method> request);


        //! @brief Record to help construct a response.
        struct HttpData {
            VersionEnum version{};
            StatusCodeEnum status{};
            string statusMessage{};
            Headers headers{};
            string body{};
        };

        using SocketPtr = std::shared_ptr<Socket>;

    private:
        // I will do it when... Idk
        // template<MethodConcept Method>
        // static expected<Response<Method>, string> _1RawSend(Request<Method> request);

        template<MethodConcept Method>
        static expected<std::pair<SocketPtr, Response<Method>>, string> _ParseHttp11(SocketPtr infoPtr);

        // template<MethodConcept Method>
        // static expected<Response<Method>, string> _2TlsSend(Request<Method> request);
        //
        // template<MethodConcept Method>
        // static expected<Response<Method>, string> _3DlsSend(Request<Method> request);
        // friend ClientJanitor;
    };
}

#include <Thoth/Http/Client.tpp>
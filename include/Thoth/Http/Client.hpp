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

    struct Socket {
        // TODO: FUTURE: Implement HTTP2 and Quic
        // using ClientSocketType = std::variant<Hermes::RawTcpClient, Hermes::RawDtlsClient>;
        Hermes::RawTlsClient socket;
        Version version;

        std::chrono::steady_clock::time_point lastUsed;
    };


    struct ClientJanitor {

        static ClientJanitor& Instance();
        void JanitorLoop();

        std::mutex poolMutex;
        std::unordered_map<Hermes::IpEndpoint, std::vector<std::shared_ptr<Socket>>> connectionPool;
    private:
        std::atomic_bool _isRunning{ true };
        ClientJanitor();
        ~ClientJanitor();

        std::jthread _janitorThread;
    };


    struct Client {
        template<MethodConcept Method = GetMethod>
        static expected<Response<Method>, string> Send(Request<Method> request);

    private:
        // I will do it when... Idk
        // template<MethodConcept Method>
        // static expected<Response<Method>, string> _1RawSend(Request<Method> request);

        // template<MethodConcept Method>
        // static expected<Response<Method>, string> _1TlsSend(Request<Method> request);
        //
        // template<MethodConcept Method>
        // static expected<Response<Method>, string> _2TlsSend(Request<Method> request);
        //
        // template<MethodConcept Method>
        // static expected<Response<Method>, string> _3DlsSend(Request<Method> request);
        // friend ClientJanitor;
    };
}

#include <Thoth/Http/Client.tpp>
#pragma once
#include <Hermes/Socket/ClientSocket.hpp>
#include <Thoth/Http/Request/HttpRequest.hpp>

#include <future>
#include <chrono>
#include <atomic>
#include <mutex>

namespace Thoth::Http {
    template<HttpMethodConcept Method>
    struct HttpResponse;

    struct HttpSocket {
        Hermes::RawTcpClient socket; // TODO: Union with UDP to implement HTTP3/Quic

        std::chrono::steady_clock::time_point lastUsed;
    };


    struct HttpClientJanitor {
        static HttpClientJanitor& Instance();
        void JanitorLoop();

        std::mutex poolMutex;
        std::unordered_map<Hermes::IpEndpoint, std::vector<std::shared_ptr<HttpSocket>>> connectionPool;
    private:
        std::atomic_bool _isRunning{ true };
        HttpClientJanitor();
        ~HttpClientJanitor();

        std::jthread _janitorThread;
    };


    struct HttpClient {
        template<HttpMethodConcept Method>
        static expected<HttpResponse<Method>, string> Send(HttpRequest<Method> request);

    private:
        friend HttpClientJanitor;
    };
}

#include <Thoth/Http/HttpClient.tpp>
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
        // TODO: FUTURE: Implement HTTP2 and Quic
        // using ClientSocketType = std::variant<Hermes::RawTcpClient, Hermes::RawDtlsClient>;
        Hermes::RawTlsClient socket;
        HttpVersion version;

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
        template<HttpMethodConcept Method = HttpGetMethod>
        static expected<HttpResponse<Method>, string> Send(HttpRequest<Method> request);

    private:
        // I will do it when... Idk
        // template<HttpMethodConcept Method>
        // static expected<HttpResponse<Method>, string> _Http1RawSend(HttpRequest<Method> request);

        // template<HttpMethodConcept Method>
        // static expected<HttpResponse<Method>, string> _Http1TlsSend(HttpRequest<Method> request);
        //
        // template<HttpMethodConcept Method>
        // static expected<HttpResponse<Method>, string> _Http2TlsSend(HttpRequest<Method> request);
        //
        // template<HttpMethodConcept Method>
        // static expected<HttpResponse<Method>, string> _Http3DlsSend(HttpRequest<Method> request);
        // friend HttpClientJanitor;
    };
}

#include <Thoth/Http/HttpClient.tpp>
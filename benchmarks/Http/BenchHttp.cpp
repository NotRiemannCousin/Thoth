/// AI GENERATED - FIXED & COMPLETE

/**
 * @file BenchHttp.cpp
 * @brief Benchmarks de clientes HTTP comparando Thoth::Http, libcurl e cpp-httplib.
 */

#include <benchmark/benchmark.h>

// ── Thoth ─────────────────────────────────────────────────────────────
#include <Thoth/Http/Methods/GetMethod.hpp>
#include <Thoth/Http/Methods/PostMethod.hpp>
#include <Thoth/Http/Client.hpp>
#include <Thoth/Http/Url/Url.hpp>

// ── cpp-httplib ────────────────────────────────────────────────────────
#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  define NOMINMAX
#endif
#define CPPHTTPLIB_OPENSSL_SUPPORT  1
#include <httplib.h>

// ── libcurl ────────────────────────────────────────────────────────────
#include <curl/curl.h>

// ── std ───────────────────────────────────────────────────────────────
#include <string>
#include <format>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <barrier>
#include <memory>

// ======================================================================
//  Configuração
// ======================================================================

static constexpr const char* kHost = BENCH_HTTP_HOST;
static constexpr int         kPort = BENCH_HTTP_PORT;
static constexpr const char* kPath = BENCH_HTTP_PATH;

static const std::string kPostBody = R"({"bench":true,"lib":"thoth","value":42})";
static constexpr const char* kPostPath = "/post";

// ======================================================================
//  Auxiliares libcurl
// ======================================================================

namespace Curl {
    static std::size_t S_SinkWrite(char*, std::size_t, std::size_t nmemb, void*) {
        return nmemb;
    }

    static std::size_t S_StringWrite(char* ptr, std::size_t, std::size_t nmemb, void* userdata) {
        reinterpret_cast<std::string*>(userdata)->append(ptr, nmemb);
        return nmemb;
    }

    struct Handle {
        CURL* h;
        Handle() : h{ curl_easy_init() } {}
        ~Handle() { if (h) curl_easy_cleanup(h); }
        Handle(const Handle&) = delete;
    };

    // Reuso explícito de conexão
    struct Session {
        CURL* h;
        explicit Session(const std::string& url) : h{ curl_easy_init() } {
            if (h) {
                curl_easy_setopt(h, CURLOPT_URL, url.c_str());
                curl_easy_setopt(h, CURLOPT_WRITEFUNCTION, S_SinkWrite);
                curl_easy_setopt(h, CURLOPT_SSL_VERIFYPEER, 0L);
                curl_easy_setopt(h, CURLOPT_SSL_VERIFYHOST, 0L);
                curl_easy_setopt(h, CURLOPT_TCP_KEEPALIVE, 1L);
            }
        }
        ~Session() { if (h) curl_easy_cleanup(h); }
        bool Perform() { return h && curl_easy_perform(h) == CURLE_OK; }
    };
}

// ======================================================================
//  Auxiliares Thoth
// ======================================================================

namespace ThothHttp {
    using namespace Thoth::Http;

    static std::string S_BaseUrl() {
        return std::format("https://{}:{}{}", kHost, kPort, kPath);
    }

    static bool S_GetWarm() {
        auto req = GetRequest::FromUrl(S_BaseUrl());
        if (!req) return false;
        auto resp = Client::Send(std::move(*req));
        return resp.has_value();
    }

    static bool S_GetCold() {
        auto req = GetRequest::FromUrl(S_BaseUrl());
        if (!req) return false;
        // Força fechar a conexão para evitar o pool automático e testar Handshake
        req->headers.Add("connection", "close");
        auto resp = Client::Send(std::move(*req));
        return resp.has_value();
    }

    static bool S_PostWarm(std::string_view body) {
        std::string url = std::format("https://{}:{}{}", kHost, kPort, kPostPath);
        auto req = PostRequest::FromUrl(url, std::string(body));
        if (!req) return false;
        auto resp = Client::Send(std::move(*req));
        return resp.has_value();
    }
}

// ======================================================================
//  CENÁRIO 1 – Cold Start (Single GET com Handshake TLS)
// ======================================================================

static void BM_Thoth_Cold_Get(benchmark::State& state) {
    for (auto _ : state) {
        bool ok = ThothHttp::S_GetCold();
        benchmark::DoNotOptimize(ok);
    }
}

static void BM_Curl_Cold_Get(benchmark::State& state) {
    std::string url = ThothHttp::S_BaseUrl();
    for (auto _ : state) {
        Curl::Handle c;
        curl_easy_setopt(c.h, CURLOPT_URL, url.c_str());
        curl_easy_setopt(c.h, CURLOPT_WRITEFUNCTION, Curl::S_SinkWrite);
        curl_easy_setopt(c.h, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(c.h, CURLOPT_FORBID_REUSE, 1L);
        bool ok = curl_easy_perform(c.h) == CURLE_OK;
        benchmark::DoNotOptimize(ok);
    }
}

static void BM_Httplib_Cold_Get(benchmark::State& state) {
    for (auto _ : state) {
        httplib::SSLClient cli(kHost, kPort);
        cli.enable_server_certificate_verification(false);
        auto res = cli.Get(kPath);
        benchmark::DoNotOptimize(res);
    }
}

// ======================================================================
//  CENÁRIO 2 – Warm Start (Single GET com Keep-Alive)
// ======================================================================

static void BM_Thoth_Warm_Get(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(ThothHttp::S_GetWarm());
    }
}

static void BM_Curl_Warm_Get(benchmark::State& state) {
    Curl::Session sess{ ThothHttp::S_BaseUrl() };
    for (auto _ : state) {
        benchmark::DoNotOptimize(sess.Perform());
    }
}

static void BM_Httplib_Warm_Get(benchmark::State& state) {
    httplib::SSLClient cli(kHost, kPort);
    cli.enable_server_certificate_verification(false);
    for (auto _ : state) {
        benchmark::DoNotOptimize(cli.Get(kPath));
    }
}

// ======================================================================
//  CENÁRIO 3 – Sequential (N pedidos seguidos na mesma iteração)
// ======================================================================

static void BM_Thoth_Sequential_Get(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));
    for (auto _ : state) {
        for (int i = 0; i < n; ++i) benchmark::DoNotOptimize(ThothHttp::S_GetWarm());
    }
    state.SetItemsProcessed(state.iterations() * n);
}

static void BM_Curl_Sequential_Get(benchmark::State& state) {
    const int n = static_cast<int>(state.range(0));
    Curl::Session sess{ ThothHttp::S_BaseUrl() };
    for (auto _ : state) {
        for (int i = 0; i < n; ++i) benchmark::DoNotOptimize(sess.Perform());
    }
    state.SetItemsProcessed(state.iterations() * n);
}

// ======================================================================
//  CENÁRIO 4 – Parallel (T threads martelando o servidor)
// ======================================================================

template<typename Fn>
static void S_RunParallel(benchmark::State& state, Fn&& work) {
    const int t = static_cast<int>(state.range(0));
    for (auto _ : state) {
        state.PauseTiming();
        std::vector<std::thread> threads;
        std::barrier ready{ t + 1 };
        for (int i = 0; i < t; ++i) {
            threads.emplace_back([&] {
                ready.arrive_and_wait();
                work();
            });
        }
        state.ResumeTiming();
        ready.arrive_and_wait();
        for (auto& thread : threads) thread.join();
    }
    state.SetItemsProcessed(state.iterations() * t);
}

static void BM_Thoth_Parallel_Get(benchmark::State& state) {
    S_RunParallel(state, [] { benchmark::DoNotOptimize(ThothHttp::S_GetWarm()); });
}

static void BM_Curl_Parallel_Get(benchmark::State& state) {
    std::string url = ThothHttp::S_BaseUrl();
    S_RunParallel(state, [&] {
        Curl::Session sess{ url };
        benchmark::DoNotOptimize(sess.Perform());
    });
}

// ======================================================================
//  CENÁRIO 5 – Body to String & POST
// ======================================================================

static void BM_Thoth_BodyToString_Get(benchmark::State& state) {
    for (auto _ : state) {
        auto req = GetRequest::FromUrl(ThothHttp::S_BaseUrl());
        auto resp = Client::Send(std::move(*req));
        std::string body = std::move(*resp).MoveBody();
        benchmark::DoNotOptimize(body);
    }
}

static void BM_Thoth_Post_WithBody(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(ThothHttp::S_PostWarm(kPostBody));
    }
}

// ======================================================================
//  Registro de Benchmarks
// ======================================================================

// Cold vs Warm
BENCHMARK(BM_Thoth_Cold_Get)   ->Name("Http/Cold/Thoth")   ->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Curl_Cold_Get)    ->Name("Http/Cold/Curl")    ->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Httplib_Cold_Get) ->Name("Http/Cold/Httplib") ->Unit(benchmark::kMillisecond);

BENCHMARK(BM_Thoth_Warm_Get)   ->Name("Http/Warm/Thoth")   ->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Curl_Warm_Get)    ->Name("Http/Warm/Curl")    ->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Httplib_Warm_Get) ->Name("Http/Warm/Httplib") ->Unit(benchmark::kMillisecond);

// Sequential
BENCHMARK(BM_Thoth_Sequential_Get)->Name("Http/Seq/Thoth")->Arg(1)->Arg(5)->Arg(10)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Curl_Sequential_Get) ->Name("Http/Seq/Curl") ->Arg(1)->Arg(5)->Arg(10)->Unit(benchmark::kMillisecond);

// Parallel
BENCHMARK(BM_Thoth_Parallel_Get)->Name("Http/Para/Thoth")->Arg(4)->Arg(8)->Arg(16)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Curl_Parallel_Get) ->Name("Http/Para/Curl") ->Arg(4)->Arg(8)->Arg(16)->Unit(benchmark::kMillisecond);

// Extras
BENCHMARK(BM_Thoth_BodyToString_Get)->Name("Http/Body/Thoth")->Unit(benchmark::kMillisecond);
BENCHMARK(BM_Thoth_Post_WithBody)   ->Name("Http/Post/Thoth")->Unit(benchmark::kMillisecond);

BENCHMARK_MAIN();
#include <gtest/gtest.h>

#include <stdexec/execution.hpp>

#include <Thoth/Http/Client/Client.hpp>
#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/Response/ResponseEntry.hpp>
#include <Thoth/Http/Server/Router.hpp>
#include <Thoth/Http/Server/Routers/BasicRouter.hpp>
#include <Thoth/Http/Server/Server.hpp>
#include <Thoth/ThothError.hpp>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

using namespace std::chrono_literals;
using namespace Thoth::Http;

namespace {
    std::atomic<std::uint16_t> k_nextPort{ 21000 };

    constexpr ClientOptions k_e2eOptions{ .requestTimeout = 5s, .connectionTimeout = 2s, .handshakeTimeout = 2s };

    struct TestServer {
        TestServer() : m_port{ k_nextPort.fetch_add(1) } {
            Hermes::DefaultSocketData<Hermes::FastIoLoop> data{{ Hermes::IpAddress::AnyIpv4(), m_port }};
            data.executionContext = &m_io;

            m_server.emplace(std::move(data));
            m_server->hosts.allowed.push_back("127.0.0.1");
        }

        ~TestServer() {
            if (m_server) m_server->Stop();
            if (m_runner.joinable()) m_runner.join();
        }

        TestServer(const TestServer&)            = delete;
        TestServer& operator=(const TestServer&) = delete;

        bool Start() {
            m_runner = std::jthread{ [this] {
                try {
                    stdexec::sync_wait(m_server->Run() | stdexec::upon_stopped([] { return 0; }));
                } catch (...) {
                    m_listenFailed.store(true, std::memory_order_release);
                }
            } };

            for (int spin{}; spin < 500; ++spin) {
                if (m_server->Running() || m_listenFailed.load(std::memory_order_acquire)) break;
                std::this_thread::sleep_for(2ms);
            }
            return m_server->Running();
        }

        // Stops the loop and waits until the listener is gone.
        void Stop() {
            if (m_server) m_server->Stop();
            if (m_runner.joinable()) m_runner.join();
        }

        [[nodiscard]] bool Running() const { return m_server->Running(); }

        [[nodiscard]] std::string Url(std::string_view path = "/") const {
            return std::format("http://127.0.0.1:{}{}", m_port, path);
        }

        [[nodiscard]] HttpLimits& Limits() { return m_server->limits; }
        [[nodiscard]] HostPolicy& Hosts() { return m_server->hosts; }

        template<class F>
        void Route(std::string path, F&& callback) {
            (*m_server)->Route(std::move(path), std::forward<F>(callback));
        }

        template<class F, class... Mws>
        void Route(std::string path, MiddlewarePack<Mws...> mws, F&& callback) {
            (*m_server)->Route(std::move(path), std::move(mws), std::forward<F>(callback));
        }

        template<class... Fs>
            requires (sizeof...(Fs) > 0)
        void Match(std::string path, Fs&&... callbacks) {
            (*m_server)->Match(std::move(path), std::forward<Fs>(callbacks)...);
        }

        template<class... Mws, class... Fs>
            requires (sizeof...(Fs) > 0)
        void Match(std::string path, MiddlewarePack<Mws...> mws, Fs&&... callbacks) {
            (*m_server)->Match(std::move(path), std::move(mws), std::forward<Fs>(callbacks)...);
        }

    private:
        Hermes::FastIoLoop      m_io{ 2 };
        std::uint16_t           m_port{};
        std::optional<Server<>> m_server;
        std::jthread            m_runner;
        std::atomic<bool>       m_listenFailed{};
    };

    template<MethodConcept Method>
    Client::ExpResponse<Method, std::string> Send(std::string_view url, std::string body = {},
            ClientOptions opts = k_e2eOptions) {
        auto request{ Request<Method>::FromUrl(url, std::move(body)) };
        if (!request) return std::unexpected{ request.error() };
        return Client::Send(std::move(*request), opts);
    }

    struct ServerE2eTest : testing::Test {
        TestServer srv;
    };
}

TEST_F(ServerE2eTest, Running_BeforeStart_IsFalse) {
    EXPECT_FALSE(srv.Running());
}

TEST_F(ServerE2eTest, Running_AfterStart_IsTrue) {
    ASSERT_TRUE(srv.Start());
    EXPECT_TRUE(srv.Running());
}

TEST_F(ServerE2eTest, Route_GetStringEntry_ReachClient) {
    srv.Route("/", [](GetRequest&) -> GetResponseEntry { return "hello e2e"; });
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<GetMethod>(srv.Url()) };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, StatusCodeEnum::Ok);
    EXPECT_EQ(res->body, "hello e2e");
    EXPECT_TRUE(res->headers.Exists("content-type"));
    EXPECT_TRUE(res->headers.Exists("content-length", "9"));
}

TEST_F(ServerE2eTest, Route_UnknownPath_Answers404) {
    srv.Route("/", [](GetRequest&) -> GetResponseEntry { return "root"; });
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<GetMethod>(srv.Url("/nowhere")) };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, StatusCodeEnum::NotFound);
    EXPECT_TRUE(res->body.empty());
}

TEST_F(ServerE2eTest, Route_UnroutedMethod_Answers405) {
    srv.Route("/", [](GetRequest&) -> GetResponseEntry { return "get"; });
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<PostMethod>(srv.Url(), "up") };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, StatusCodeEnum::MethodNotAllowed);
}

TEST_F(ServerE2eTest, Match_SamePathPerMethod_EachCallbackAnswers) {
    srv.Match("/multi",
        [](GetRequest&)  -> GetResponseEntry  { return "from get";  },
        [](PostRequest&) -> PostResponseEntry { return "from post"; });
    ASSERT_TRUE(srv.Start());

    const auto get{ Send<GetMethod>(srv.Url("/multi")) };
    ASSERT_TRUE(get);
    EXPECT_EQ(get->body, "from get");

    const auto post{ Send<PostMethod>(srv.Url("/multi"), "x") };
    ASSERT_TRUE(post);
    EXPECT_EQ(post->body, "from post");
}

TEST_F(ServerE2eTest, Route_PostEcho_BodyRoundTrips) {
    srv.Route("/echo", [](PostRequest& request) -> PostResponseEntry { return std::move(request.body); });
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<PostMethod>(srv.Url("/echo"), "bunnies") };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, StatusCodeEnum::Ok);
    EXPECT_EQ(res->body, "bunnies");
}

TEST_F(ServerE2eTest, Route_CustomStatus_NoContent) {
    srv.Route("/gone", [](PostRequest&) -> PostResponse {
        return {{ .status = StatusCodeEnum::NoContent }};
    });
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<PostMethod>(srv.Url("/gone"), "gone") };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, StatusCodeEnum::NoContent);
    EXPECT_TRUE(res->body.empty());
}

TEST_F(ServerE2eTest, Route_HandlerThrows_AnswersThrownCode) {
    srv.Route("/teapot", [](GetRequest&) -> GetResponseEntry {
        throw StatusCodeEnum::ImATeapot;
    });
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<GetMethod>(srv.Url("/teapot")) };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, StatusCodeEnum::ImATeapot);
    EXPECT_TRUE(res->body.empty());
}

TEST_F(ServerE2eTest, Route_ResponseHeader_ReachesClient) {
    srv.Route("/meta", [](GetRequest&) -> GetResponse {
        GetResponse response{};
        response.headers.Set("x-thoth", "e2e");
        response.body = "meta";
        return response;
    });
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<GetMethod>(srv.Url("/meta")) };
    ASSERT_TRUE(res);
    EXPECT_TRUE(res->headers.Exists("x-thoth", "e2e"));
    EXPECT_EQ(res->body, "meta");
}

TEST_F(ServerE2eTest, Route_RequestHeader_ReachesHandler) {
    srv.Route("/secret", [](const GetRequest& request) -> GetResponseEntry {
        const auto secret{ request.headers.Get("x-secret") };
        return secret ? **secret : std::string{ "missing" };
    });
    ASSERT_TRUE(srv.Start());

    auto request{ GetRequest::FromUrl(srv.Url("/secret")) };
    ASSERT_TRUE(request);
    request->headers.Set("x-secret", "s3cr3t");

    const auto res{ Client::Send(std::move(*request), k_e2eOptions) };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->body, "s3cr3t");
}

TEST_F(ServerE2eTest, Route_QueryParams_ReachHandler) {
    srv.Route("/hello", [](const GetRequest& request) -> GetResponseEntry {
        if (const auto query{ request.url.GetQueryParams() })
            if (const auto name{ query->GetCopyFrontNonEmpty("name") })
                return std::format("hi {}", *name);
        return "hi stranger";
    });
    ASSERT_TRUE(srv.Start());

    const auto plain{ Send<GetMethod>(srv.Url("/hello?name=thoth")) };
    ASSERT_TRUE(plain);
    EXPECT_EQ(plain->body, "hi thoth");

    const auto encoded{ Send<GetMethod>(srv.Url("/hello?name=two%20words")) };
    ASSERT_TRUE(encoded);
    EXPECT_EQ(encoded->body, "hi two words");

    const auto none{ Send<GetMethod>(srv.Url("/hello")) };
    ASSERT_TRUE(none);
    EXPECT_EQ(none->body, "hi stranger");
}

TEST_F(ServerE2eTest, Route_QueryMultiValue_CountedByHandler) {
    srv.Route("/count", [](const GetRequest& request) -> GetResponseEntry {
        const auto query{ request.url.GetQueryParams() };
        const auto tags{ query ? query->GetCopy("tag") : std::nullopt };
        return std::format("{} tags", tags ? tags->size() : std::size_t{ 0 });
    });
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<GetMethod>(srv.Url("/count?tag=cpp&tag=http&tag=e2e")) };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->body, "3 tags");
}

TEST_F(ServerE2eTest, Route_MiddlewareRequired_ValueReachesHandler) {
    struct Session { std::string user; };
    const auto session{ [](const auto&) -> std::expected<Session, StatusCodeEnum> {
        return Session{ "thoth" };
    } };

    srv.Route("/who", Use(session), [](const GetRequest&, Session who) -> GetResponseEntry {
        return std::format("user:{}", who.user);
    });
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<GetMethod>(srv.Url("/who")) };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->body, "user:thoth");
}

TEST_F(ServerE2eTest, Route_MiddlewareOptional_NulloptWhenHeaderAbsent) {
    struct Login { std::string user; };
    const auto login{ [](const auto& exchange) -> std::optional<Login> {
        if (const auto user{ exchange.head.headers.Get("x-user") }) return Login{ **user };
        return std::nullopt;
    } };

    srv.Route("/maybe", Use(login), [](const GetRequest&, const std::optional<Login>& lgn) -> GetResponseEntry {
        return lgn ? "logged:" + lgn->user : "anonymous";
    });
    ASSERT_TRUE(srv.Start());

    auto request{ GetRequest::FromUrl(srv.Url("/maybe")) };
    ASSERT_TRUE(request);
    request->headers.Set("x-user", "thoth");

    const auto withUser{ Client::Send(std::move(*request), k_e2eOptions) };
    ASSERT_TRUE(withUser);
    EXPECT_EQ(withUser->body, "logged:thoth");

    const auto anonymous{ Send<GetMethod>(srv.Url("/maybe")) };
    ASSERT_TRUE(anonymous);
    EXPECT_EQ(anonymous->body, "anonymous");
}

TEST_F(ServerE2eTest, Route_MiddlewareError_AnswersMappedStatus) {
    struct Gate {};
    const auto guard{ [](const auto&) -> std::expected<Gate, StatusCodeEnum> {
        return std::unexpected{ StatusCodeEnum::Unauthorized };
    } };

    srv.Route("/admin", Use(guard), [](const GetRequest&, Gate) -> GetResponseEntry {
        return "never";
    });
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<GetMethod>(srv.Url("/admin")) };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, StatusCodeEnum::Unauthorized);
    EXPECT_TRUE(res->body.empty());
}

TEST_F(ServerE2eTest, Route_MiddlewareStack_HandlerSeesBoth) {
    struct Who { std::string name; };
    struct Target { std::string id; };

    const auto who{ [](const auto&) -> std::expected<Who, StatusCodeEnum> {
        return Who{ "thoth" };
    } };
    const auto target{ [](const auto& exchange) -> std::expected<Target, StatusCodeEnum> {
        if (const auto query{ exchange.head.url.GetQueryParams() })
            if (const auto id{ query->GetCopyFrontNonEmpty("id") })
                return Target{ *id };
        return Target{ "0" };
    } };

    srv.Route("/combo", Use(who, target),
        [](const GetRequest&, Who who, Target target) -> GetResponseEntry {
            return std::format("{}:{}", who.name, target.id);
        });
    ASSERT_TRUE(srv.Start());

    const auto withId{ Send<GetMethod>(srv.Url("/combo?id=7")) };
    ASSERT_TRUE(withId);
    EXPECT_EQ(withId->body, "thoth:7");

    const auto noId{ Send<GetMethod>(srv.Url("/combo")) };
    ASSERT_TRUE(noId);
    EXPECT_EQ(noId->body, "thoth:0");
}

TEST_F(ServerE2eTest, Hosts_EmptyAllowedList_DropsConnection) {
    srv.Hosts().allowed.clear();
    srv.Route("/", [](GetRequest&) -> GetResponseEntry { return "nope"; });
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<GetMethod>(srv.Url()) };
    ASSERT_FALSE(res);
    EXPECT_TRUE(res.error().Is<MessageParseErrorEnum>());
}

TEST_F(ServerE2eTest, Limits_MaxRequestsPerConnection_ClosesOnCap) {
    srv.Route("/", [](GetRequest&) -> GetResponseEntry { return "ok"; });
    srv.Limits().maxRequestsPerConnection = 2;
    ASSERT_TRUE(srv.Start());

    const auto first{  Send<GetMethod>(srv.Url()) };
    const auto second{ Send<GetMethod>(srv.Url()) };
    const auto third{  Send<GetMethod>(srv.Url()) };

    ASSERT_TRUE(first);
    ASSERT_TRUE(second);
    ASSERT_TRUE(third);
    EXPECT_FALSE(first ->headers.Exists("connection", "close"));
    EXPECT_FALSE(second->headers.Exists("connection", "close"));
    EXPECT_TRUE(third  ->headers.Exists("connection", "close"));
}

TEST_F(ServerE2eTest, Limits_ContentLengthOverMax_DropsConnection) {
    srv.Route("/echo", [](PostRequest& request) -> PostResponseEntry { return std::move(request.body); });
    srv.Limits().maxBodyBytes = 8;
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<PostMethod>(srv.Url("/echo"), std::string(32, 'x')) };
    ASSERT_FALSE(res);
    EXPECT_TRUE(res.error().Is<MessageParseErrorEnum>());
}

TEST_F(ServerE2eTest, KeepAlive_TenSequentialRequests_AllSucceed) {
    srv.Route("/", [](GetRequest&) -> GetResponseEntry { return "pong"; });
    ASSERT_TRUE(srv.Start());

    for (int i{}; i < 10; ++i) {
        const auto res{ Send<GetMethod>(srv.Url()) };
        ASSERT_TRUE(res);
        EXPECT_EQ(res->body, "pong");
    }
}

TEST_F(ServerE2eTest, Stop_RunningFlagFlipsAndNextRequestFails) {
    srv.Route("/", [](GetRequest&) -> GetResponseEntry { return "ok"; });
    ASSERT_TRUE(srv.Start());
    EXPECT_TRUE(srv.Running());

    srv.Stop();
    srv.Stop(); // idempotent
    EXPECT_FALSE(srv.Running());

    const auto res{ Send<GetMethod>(srv.Url()) };
    EXPECT_FALSE(res);
    EXPECT_TRUE(res.error().Is<ConnectionErrorEnum>());
}

TEST_F(ServerE2eTest, Concurrent_Clients_AllRequestsServed) {
    std::atomic<std::size_t> hits{};
    srv.Route("/", [&hits](GetRequest&) -> GetResponseEntry {
        return std::format("hit {}", hits.fetch_add(1) + 1);
    });
    ASSERT_TRUE(srv.Start());

    constexpr std::size_t k_threads{ 4 };
    constexpr std::size_t k_perThread{ 5 };
    std::atomic<std::size_t> failures{};

    std::vector<std::thread> workers;
    for (std::size_t t{}; t < k_threads; ++t)
        workers.emplace_back([&] {
            for (std::size_t i{}; i < k_perThread; ++i)
                if (const auto res{ Send<GetMethod>(srv.Url()) }; !res) ++failures;
        });

    for (auto& worker : workers) worker.join();

    EXPECT_EQ(failures.load(), std::size_t{});
    EXPECT_EQ(hits.load(), k_threads * k_perThread);
}

#include <gtest/gtest.h>

#include <stdexec/execution.hpp>

#include <Thoth/Http/Client/Client.hpp>
#include <Thoth/Http/Methods/DeleteMethod.hpp>
#include <Thoth/Http/Methods/HeadMethod.hpp>
#include <Thoth/Http/Methods/PatchMethod.hpp>
#include <Thoth/Http/Methods/PutMethod.hpp>
#include <Thoth/Http/NHeaders/Headers/MimeType.hpp>
#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/Response/ResponseEntry.hpp>
#include <Thoth/Http/Server/Router.hpp>
#include <Thoth/Http/Server/Routers/BasicRouter.hpp>
#include <Thoth/Http/Server/Server.hpp>
#include <Thoth/NJson/Json.hpp>
#include <Thoth/NJson/JsonObject.hpp>
#include <Thoth/ThothError.hpp>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

using namespace std::chrono_literals;
using namespace Thoth::Http;
using namespace Thoth::NJson;

namespace {
    std::atomic<std::uint16_t> k_nextPort{ 21500 };

    constexpr ClientOptions k_e2eOptions{ .requestTimeout = 5s, .connectionTimeout = 2s, .handshakeTimeout = 2s };

    using PutRequest    = Request<PutMethod>;
    using PatchRequest  = Request<PatchMethod>;
    using DeleteRequest = Request<DeleteMethod>;
    using HeadRequest   = Request<HeadMethod>;

    using PutResponseEntry    = ResponseEntry<PutMethod>;
    using PatchResponseEntry  = ResponseEntry<PatchMethod>;
    using DeleteResponseEntry = ResponseEntry<DeleteMethod>;
    using HeadResponseEntry   = ResponseEntry<HeadMethod>;

    struct TestServer {
        TestServer() : m_port{ k_nextPort.fetch_add(1) } {
            Hermes::DefaultSocketData<Hermes::FastIoLoop> data{{ Hermes::IpAddress::LoopbackIpv4(), m_port }};
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

        [[nodiscard]] std::string Url(std::string_view path = "/") const {
            return std::format("http://127.0.0.1:{}{}", m_port, path);
        }

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

    struct HttpE2eTest : testing::Test {
        TestServer srv;
    };
}

TEST_F(HttpE2eTest, Get_RoundTrip_StatusBodyAndHeaders) {
    srv.Route("/ping", [](GetRequest&) -> GetResponseEntry { return "pong"; });
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<GetMethod>(srv.Url("/ping")) };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, StatusCodeEnum::Ok);
    EXPECT_EQ(res->body, "pong");
    EXPECT_EQ(res->version, VersionEnum::Http1_1);
    EXPECT_TRUE(res->headers.Exists("content-type", "text/plain"));
    EXPECT_TRUE(res->headers.Exists("content-length", "4"));
}

TEST_F(HttpE2eTest, Get_SuccessfulFlag_FollowsStatusClass) {
    srv.Route("/accepted", [](GetRequest&) -> GetResponse {
        return { { .status = StatusCodeEnum::Accepted }, "queued" };
    });
    srv.Route("/missing", [](GetRequest&) -> GetResponseEntry { return "never"; });
    ASSERT_TRUE(srv.Start());

    const auto ok{ Send<GetMethod>(srv.Url("/accepted")) };
    ASSERT_TRUE(ok);
    EXPECT_EQ(ok->status, StatusCodeEnum::Accepted);
    EXPECT_TRUE(ok->Successful());

    const auto missing{ Send<GetMethod>(srv.Url("/nope")) };
    ASSERT_TRUE(missing);
    EXPECT_FALSE(missing->Successful());
}

TEST_F(HttpE2eTest, Post_Echo_SmallBody) {
    srv.Route("/echo", [](PostRequest& request) -> PostResponseEntry { return std::move(request.body); });
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<PostMethod>(srv.Url("/echo"), "bunnies") };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, StatusCodeEnum::Ok);
    EXPECT_EQ(res->body, "bunnies");
}

TEST_F(HttpE2eTest, Post_Echo_LargeBodyIntact) {
    srv.Route("/echo", [](PostRequest& request) -> PostResponseEntry { return std::move(request.body); });
    ASSERT_TRUE(srv.Start());

    std::string big{};
    big.reserve(100'000);
    for (int i{}; i < 100'000; ++i) big.push_back(static_cast<char>('a' + i % 26));

    const auto res{ Send<PostMethod>(srv.Url("/echo"), big) };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->body, big);
    EXPECT_TRUE(res->headers.Exists("content-length", "100000"));
}

TEST_F(HttpE2eTest, Post_Echo_EmptyBody) {
    srv.Route("/echo", [](PostRequest& request) -> PostResponseEntry { return std::move(request.body); });
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<PostMethod>(srv.Url("/echo"), std::string{}) };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, StatusCodeEnum::Ok);
    EXPECT_TRUE(res->body.empty());
    EXPECT_TRUE(res->headers.Exists("content-length", "0"));
}

TEST_F(HttpE2eTest, Methods_Match_PutPatchDeleteOnSamePath) {
    srv.Match("/res",
        [](PutRequest& request)   -> PutResponseEntry    { return "put:"   + std::move(request.body); },
        [](PatchRequest& request) -> PatchResponseEntry  { return "patch:" + std::move(request.body); },
        [](DeleteRequest&)        -> DeleteResponseEntry { return "deleted"; });
    ASSERT_TRUE(srv.Start());

    const auto put{ Send<PutMethod>(srv.Url("/res"), "v2") };
    ASSERT_TRUE(put);
    EXPECT_EQ(put->body, "put:v2");

    const auto patch{ Send<PatchMethod>(srv.Url("/res"), "diff") };
    ASSERT_TRUE(patch);
    EXPECT_EQ(patch->body, "patch:diff");

    const auto del{ Send<DeleteMethod>(srv.Url("/res")) };
    ASSERT_TRUE(del);
    EXPECT_EQ(del->body, "deleted");

    const auto get{ Send<GetMethod>(srv.Url("/res")) };
    ASSERT_TRUE(get);
    EXPECT_EQ(get->status, StatusCodeEnum::MethodNotAllowed);
}

TEST_F(HttpE2eTest, Methods_HeadRequest_Served) {
    srv.Match("/",
        [](GetRequest&)   -> GetResponseEntry  { return "get";  },
        [](HeadRequest&)  -> HeadResponseEntry { return "head"; });
    ASSERT_TRUE(srv.Start());

    const auto head{ Send<HeadMethod>(srv.Url()) };
    ASSERT_TRUE(head);
    EXPECT_EQ(head->status, StatusCodeEnum::Ok);
    EXPECT_EQ(head->body, "head");
}

TEST_F(HttpE2eTest, Url_Query_IgnoredByRouting) {
    srv.Route("/ping", [](GetRequest&) -> GetResponseEntry { return "pong"; });
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<GetMethod>(srv.Url("/ping?verbose=yes&debug=1")) };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->body, "pong");
}

TEST_F(HttpE2eTest, Url_Path_MatchesExactly) {
    srv.Route("/a", [](GetRequest&) -> GetResponseEntry { return "a"; });
    ASSERT_TRUE(srv.Start());

    const auto exact{ Send<GetMethod>(srv.Url("/a")) };
    ASSERT_TRUE(exact);
    EXPECT_EQ(exact->status, StatusCodeEnum::Ok);

    const auto trailingSlash{ Send<GetMethod>(srv.Url("/a/")) };
    ASSERT_TRUE(trailingSlash);
    EXPECT_EQ(trailingSlash->status, StatusCodeEnum::NotFound);

    const auto upperCase{ Send<GetMethod>(srv.Url("/A")) };
    ASSERT_TRUE(upperCase);
    EXPECT_EQ(upperCase->status, StatusCodeEnum::NotFound);
}

TEST_F(HttpE2eTest, Json_RoundTrip_BothEnds) {
    srv.Route("/api/echo", [](const PostRequest& request) -> PostResponseEntry {
        return request.AsJson().transform([](const Json& json) {
            const auto text{ json.Get("text")
                    .and_then(&Json::EnsureCpy<String>)
                    .value_or(String{}) };
            return Json{ JsonObject{ { "echo", Json{ text } } } };
        });
    });
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<PostMethod>(srv.Url("/api/echo"), R"({"text":"thoth"})") };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, StatusCodeEnum::Ok);
    EXPECT_TRUE(res->headers.Exists("content-type", "application/json"));

    const auto parsed{ res->AsJson() };
    ASSERT_TRUE(parsed);
    EXPECT_EQ(parsed->Get("echo")
            .and_then(&Json::EnsureCpy<String>)
            .transform(&String::AsCopy)->view(), "thoth");
}

TEST_F(HttpE2eTest, Json_MalformedBody_Answers400) {
    srv.Route("/api/echo", [](const PostRequest& request) -> PostResponseEntry {
        return request.AsJson().transform([](const Json& json) {
            const auto text{ json.Get("text")
                    .and_then(&Json::EnsureCpy<String>)
                    .value_or(String{}) };
            return Json{ JsonObject{ { "echo", Json{ text } } } };
        });
    });
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<PostMethod>(srv.Url("/api/echo"), "not json at all") };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, StatusCodeEnum::BadRequest);
    EXPECT_FALSE(res->body.empty()); // the parse error text
}

TEST_F(HttpE2eTest, Bytes_BinaryRoundTrip) {
    srv.Route("/bin", [](PostBinRequest& request) -> PostBinResponseEntry {
        return std::move(request.body);
    });
    ASSERT_TRUE(srv.Start());

    std::vector<std::byte> payload(512);
    for (std::size_t i{}; i < payload.size(); ++i) payload[i] = static_cast<std::byte>(i);

    auto request{ PostBinRequest::FromUrl(srv.Url("/bin"), payload) };
    ASSERT_TRUE(request);

    const auto res{ Client::Send(std::move(*request), k_e2eOptions) };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->body, payload);
    EXPECT_TRUE(res->headers.Exists("content-type", "application/octet-stream"));
}

TEST_F(HttpE2eTest, File_DownloadRoundTrip) {
    namespace fs = std::filesystem;

    std::string contents(8'192, 'x');
    for (std::size_t i{}; i < contents.size(); i += 64) contents[i] = static_cast<char>('0' + i / 64 % 10);

    const fs::path source{ fs::temp_directory_path() / "thoth_e2e_source.bin" };
    {
        std::ofstream out{ source, std::ios::binary };
        out.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    }

    srv.Route("/file", [source](GetRequest&) -> GetFileResponse {
        GetFileResponse response{};
        response.headers.ContentType() = NHeaders::MimeTypes::textPlain;
        response.body = FileBody{ source };
        return response;
    });
    ASSERT_TRUE(srv.Start());

    const fs::path target{ fs::temp_directory_path() / "thoth_e2e_download.bin" };
    auto request{ GetRequest::FromUrl(srv.Url("/file")) };
    ASSERT_TRUE(request);

    const auto res{ Client::SendAsAndParse<GetMethod, std::string, FileBody>(
            std::move(*request), FileBody::H_AsBody(FileBuilderParams{ .path = target.string() }), k_e2eOptions) };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, StatusCodeEnum::Ok);

    std::ifstream in{ target, std::ios::binary };
    std::ostringstream downloaded;
    downloaded << in.rdbuf();
    EXPECT_EQ(downloaded.str(), contents);
}

TEST_F(HttpE2eTest, File_DownloadOverLimit_ClientRefuses) {
    namespace fs = std::filesystem;

    std::string contents(1'024, 'x');
    const fs::path source{ fs::temp_directory_path() / "thoth_e2e_limit.bin" };
    {
        std::ofstream out{ source, std::ios::binary };
        out.write(contents.data(), static_cast<std::streamsize>(contents.size()));
    }

    srv.Route("/file", [source](GetRequest&) -> GetFileResponse {
        GetFileResponse response{};
        response.headers.ContentType() = NHeaders::MimeTypes::textPlain;
        response.body = FileBody{ source };
        return response;
    });
    ASSERT_TRUE(srv.Start());

    const fs::path target{ fs::temp_directory_path() / "thoth_e2e_refused.bin" };
    auto request{ GetRequest::FromUrl(srv.Url("/file")) };
    ASSERT_TRUE(request);

    // the factory rejects the head before any body byte is consumed
    const auto res{ Client::SendAsAndParse<GetMethod, std::string, FileBody>(
            std::move(*request),
            FileBody::H_AsBody(FileBuilderParams{ .path = target.string(), .maxSize = 4 }),
            k_e2eOptions) };
    ASSERT_FALSE(res);
    EXPECT_TRUE(res.error().Is<Thoth::GenericError>());
}

TEST_F(HttpE2eTest, Status_CreatedWithLocation) {
    srv.Route("/items", [](const PostRequest&) -> PostResponse {
        PostResponse response{{ .status = StatusCodeEnum::Created }};
        response.headers.Set("location", "/items/42");
        response.body = "made";
        return response;
    });
    ASSERT_TRUE(srv.Start());

    const auto res{ Send<PostMethod>(srv.Url("/items"), "new item") };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, StatusCodeEnum::Created);
    EXPECT_TRUE(res->headers.Exists("location", "/items/42"));
    EXPECT_EQ(res->body, "made");
}

TEST_F(HttpE2eTest, Client_RequestTimeout_SlowRoute_ReturnsReceiveTimeout) {
    srv.Route("/slow", [](GetRequest&) -> GetResponseEntry {
        std::this_thread::sleep_for(300ms);
        return "late";
    });
    ASSERT_TRUE(srv.Start());

    auto options{ k_e2eOptions };
    options.requestTimeout = 100ms;

    const auto res{ Send<GetMethod>(srv.Url("/slow"), std::string{}, options) };
    ASSERT_FALSE(res);
    EXPECT_EQ(res.error(), ConnectionErrorEnum::ReceiveTimeout);
}

TEST_F(HttpE2eTest, Client_NothingListening_ReturnsConnectionFailed) {
    // the fixture's server exists but was never started
    const auto res{ Send<GetMethod>(srv.Url()) };
    ASSERT_FALSE(res);
    EXPECT_EQ(res.error(), ConnectionErrorEnum::ConnectionFailed);
}

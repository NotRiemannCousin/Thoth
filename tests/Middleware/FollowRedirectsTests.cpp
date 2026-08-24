#include <gtest/gtest.h>
#include <Thoth/Http/Middleware/FollowRedirects.hpp>
#include <Thoth/Http/Client/Client.hpp>
#include <Thoth/ThothError.hpp>

#include <chrono>
#include <vector>

using namespace Thoth::Http;
using namespace std::chrono_literals;

namespace {
    using ReqT  = Request<GetMethod, std::string>;
    using RespT = Response<GetMethod, std::string>;

    constexpr ClientOptions k_networkOptions{ .connectionTimeout = 5s, .handshakeTimeout = 5s };

    template<class Result>
    bool IsUnavailable(const Result& result) {
        static constexpr ConnectionErrorEnum k_values[] {
            ConnectionErrorEnum::ConnectionFailed       , ConnectionErrorEnum::ConnectionTimeout,
            ConnectionErrorEnum::HandshakeFailed        , ConnectionErrorEnum::CertificateError,
            ConnectionErrorEnum::EncryptionFailed       , ConnectionErrorEnum::InvalidSecurityContext,
            ConnectionErrorEnum::ResolveHostNotFound    , ConnectionErrorEnum::ResolveServiceNotFound,
            ConnectionErrorEnum::ResolveTemporaryFailure, ConnectionErrorEnum::ResolveFailed,
            ConnectionErrorEnum::ResolveNoAddressFound  , ConnectionErrorEnum::UnsupportedAddressFamily
        };

        if (result || !result.error().template Is<ConnectionErrorEnum>()) return false;
        return std::ranges::contains(k_values, result.error().template As<ConnectionErrorEnum>());
    }

    RespT MakeResponse(StatusCodeEnum status, Headers headers = {}, std::string body = {}) {
        RespT r{};
        r.status  = status;
        r.headers = { std::move(headers) };
        r.body    = std::move(body);
        return r;
    }
}

#pragma region Mocks
struct FollowRedirectsTest : testing::Test {
    std::vector<ReqT> calls;

    auto ScriptedNext(std::vector<Client::ExpResponse<GetMethod, std::string>> responses) {
        return [this, responses = std::move(responses), i = std::size_t{}](ReqT request) mutable
            -> Client::ExpResponse<GetMethod, std::string> {
            calls.push_back(request);
            return responses.at(i++);
        };
    }
};

TEST_F(FollowRedirectsTest, NonRedirectResponse_PassesThroughUnchanged) {
    auto pipeline{ FollowRedirects(ScriptedNext({ MakeResponse(StatusCodeEnum::Ok, {}, "hi") })) };
    auto request{ ReqT::FromUrl("https://example.test/a") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, StatusCodeEnum::Ok);
    EXPECT_EQ(result->body, "hi");
    EXPECT_EQ(calls.size(), 1u);
}

TEST_F(FollowRedirectsTest, TemporaryRedirect_FollowsToFinalResponse) {
    auto pipeline{ FollowRedirects(ScriptedNext({
        MakeResponse(StatusCodeEnum::TemporaryRedirect, { { "location", "https://example.test/b" } }),
        MakeResponse(StatusCodeEnum::Ok, {}, "final")
    })) };
    auto request{ ReqT::FromUrl("https://example.test/a") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, StatusCodeEnum::Ok);
    EXPECT_EQ(result->body, "final");
    ASSERT_EQ(calls.size(), 2u);
    EXPECT_EQ(calls[1].url.GetPath(), "/b");
}

TEST_F(FollowRedirectsTest, PermanentRedirect_AlsoFollowed) {
    auto pipeline{ FollowRedirects(ScriptedNext({
        MakeResponse(StatusCodeEnum::PermanentRedirect, { { "location", "/b" } }),
        MakeResponse(StatusCodeEnum::Ok)
    })) };
    auto request{ ReqT::FromUrl("https://example.test/a") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, StatusCodeEnum::Ok);
}

TEST_F(FollowRedirectsTest, RelativeLocation_ResolvedAgainstCurrentUrl) {
    auto pipeline{ FollowRedirects(ScriptedNext({
        MakeResponse(StatusCodeEnum::TemporaryRedirect, { { "location", "../c" } }),
        MakeResponse(StatusCodeEnum::Ok)
    })) };
    auto request{ ReqT::FromUrl("https://example.test/a/b") };
    ASSERT_TRUE(request);

    (void)pipeline(std::move(*request));
    ASSERT_EQ(calls.size(), 2u);
    EXPECT_EQ(calls[1].url.GetPath(), "/c");
}

TEST_F(FollowRedirectsTest, MissingLocation_ReturnsRedirectResponseUnchanged) {
    auto pipeline{ FollowRedirects(ScriptedNext({
        MakeResponse(StatusCodeEnum::TemporaryRedirect)
    })) };
    auto request{ ReqT::FromUrl("https://example.test/a") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, StatusCodeEnum::TemporaryRedirect);
    EXPECT_EQ(calls.size(), 1u);
}

TEST_F(FollowRedirectsTest, ExceedsMaxHops_ReturnsError) {
    std::vector<Client::ExpResponse<GetMethod, std::string>> responses;
    for (int i{}; i < 5; ++i)
        responses.push_back(MakeResponse(StatusCodeEnum::TemporaryRedirect, { { "location", "/next" } }));

    auto pipeline{ FollowRedirects(ScriptedNext(std::move(responses)), /*maxHops=*/3) };
    auto request{ ReqT::FromUrl("https://example.test/a") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_FALSE(result);
    EXPECT_EQ(calls.size(), 3u);
}

TEST_F(FollowRedirectsTest, TransportError_PropagatedWithoutFollowing) {
    auto pipeline{ FollowRedirects([this](ReqT request) -> Client::ExpResponse<GetMethod, std::string> {
        calls.push_back(request);
        return std::unexpected{ Thoth::ThothError{ ConnectionErrorEnum::ConnectionFailed } };
    }) };
    auto request{ ReqT::FromUrl("https://example.test/a") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_FALSE(result);
    EXPECT_EQ(calls.size(), 1u);
}

TEST_F(FollowRedirectsTest, CrossOriginRedirect_StripsAuthorizationAndCookie) {
    auto pipeline{ FollowRedirects(ScriptedNext({
        MakeResponse(StatusCodeEnum::TemporaryRedirect, { { "location", "https://other.test/b" } }),
        MakeResponse(StatusCodeEnum::Ok)
    })) };
    auto request{ ReqT::FromUrl("https://example.test/a") };
    ASSERT_TRUE(request);
    request->headers.Set("authorization", "Bearer secret");
    request->headers.Set("cookie", "session=abc");

    (void)pipeline(std::move(*request));
    ASSERT_EQ(calls.size(), 2u);
    EXPECT_FALSE(calls[1].headers.Exists("authorization"));
    EXPECT_FALSE(calls[1].headers.Exists("cookie"));
}

TEST_F(FollowRedirectsTest, SameOriginRedirect_PreservesAuthorization) {
    auto pipeline{ FollowRedirects(ScriptedNext({
        MakeResponse(StatusCodeEnum::TemporaryRedirect, { { "location", "/b" } }),
        MakeResponse(StatusCodeEnum::Ok)
    })) };
    auto request{ ReqT::FromUrl("https://example.test/a") };
    ASSERT_TRUE(request);
    request->headers.Set("authorization", "Bearer secret");

    (void)pipeline(std::move(*request));
    ASSERT_EQ(calls.size(), 2u);
    EXPECT_TRUE(calls[1].headers.Exists("authorization", "Bearer secret"));
}

TEST_F(FollowRedirectsTest, Redirect_UpdatesHostHeaderToNewAuthority) {
    auto pipeline{ FollowRedirects(ScriptedNext({
        MakeResponse(StatusCodeEnum::TemporaryRedirect, { { "location", "https://other.test/b" } }),
        MakeResponse(StatusCodeEnum::Ok)
    })) };
    auto request{ ReqT::FromUrl("https://example.test/a") };
    ASSERT_TRUE(request);

    (void)pipeline(std::move(*request));
    ASSERT_EQ(calls.size(), 2u);
    EXPECT_TRUE(calls[1].headers.Exists("host", "other.test"));
}

TEST_F(FollowRedirectsTest, RequestBody_ResentUnchangedOnHop) {
    auto pipeline{ FollowRedirects(ScriptedNext({
        MakeResponse(StatusCodeEnum::TemporaryRedirect, { { "location", "/b" } }),
        MakeResponse(StatusCodeEnum::Ok)
    })) };
    auto request{ ReqT::FromUrl("https://example.test/a") };
    ASSERT_TRUE(request);
    request->body = "same body every hop";

    (void)pipeline(std::move(*request));
    ASSERT_EQ(calls.size(), 2u);
    EXPECT_EQ(calls[0].body, "same body every hop");
    EXPECT_EQ(calls[1].body, "same body every hop");
}
#pragma endregion

#pragma region Integration
TEST_F(FollowRedirectsTest, Integration_HttpBinRedirect_FollowsToDestination) {
    auto request{ ReqT::FromUrl("https://httpbin.org/redirect-to?url=https%3A%2F%2Fhttpbin.org%2Fget&status_code=307") };
    ASSERT_TRUE(request.has_value());

    auto pipeline{ FollowRedirects([](ReqT req) { return Client::Send(std::move(req), k_networkOptions); }) };

    const auto result{ pipeline(std::move(*request)) };
    if (IsUnavailable(result)) GTEST_SKIP() << "httpbin.org is unavailable";

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status, StatusCodeEnum::Ok);
    EXPECT_TRUE(result->body.find("\"https://httpbin.org/get\"") != std::string::npos);
}
#pragma endregion
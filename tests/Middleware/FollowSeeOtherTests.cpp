#include <gtest/gtest.h>
#include <Thoth/Http/Middleware/FollowSeeOther.hpp>
#include <Thoth/Http/Client/Client.hpp>
#include <Thoth/Http/Methods/PostMethod.hpp>
#include <Thoth/ThothError.hpp>

#include <chrono>

using namespace Thoth::Http;
using namespace std::chrono_literals;

namespace {
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
}

struct FollowSeeOtherTest : testing::Test {};

#pragma region Mocks
TEST_F(FollowSeeOtherTest, PostWith303_DowngradesToGetWithEmptyBody) {
    int postCalls{}, getCalls{};
    std::string getBody{ "not empty" }, getUrlPath;

    auto next{ [&]<MethodConcept M>(Request<M, std::string> request) -> Client::ExpResponse<M, std::string> {
        if constexpr (std::same_as<M, PostMethod>) {
            ++postCalls;
            Response<M, std::string> r{};
            r.status = StatusCodeEnum::SeeOther;
            r.headers.Set("location", "https://example.test/result");
            return r;
        } else {
            ++getCalls;
            getBody    = request.body;
            getUrlPath = request.url.GetPath();
            Response<M, std::string> r{};
            r.status = StatusCodeEnum::Ok;
            r.body   = "done";
            return r;
        }
    } };

    auto pipeline{ FollowSeeOther(next) };
    auto request{ Request<PostMethod, std::string>::FromUrl("https://example.test/create", "payload") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, StatusCodeEnum::Ok);
    EXPECT_EQ(result->body, "done");
    EXPECT_EQ(postCalls, 1);
    EXPECT_EQ(getCalls, 1);
    EXPECT_TRUE(getBody.empty());
    EXPECT_EQ(getUrlPath, "/result");
}

TEST_F(FollowSeeOtherTest, GetWith303_StaysGet) {
    int calls{};
    std::string lastPath;

    auto next{ [&](Request<GetMethod, std::string> request) -> Client::ExpResponse<GetMethod, std::string> {
        ++calls;
        lastPath = request.url.GetPath();
        Response<GetMethod, std::string> r{};
        if (calls == 1) {
            r.status = StatusCodeEnum::SeeOther;
            r.headers.Set("location", "/other");
        } else {
            r.status = StatusCodeEnum::Ok;
        }
        return r;
    } };

    auto pipeline{ FollowSeeOther(next) };
    auto request{ Request<GetMethod, std::string>::FromUrl("https://example.test/start") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, StatusCodeEnum::Ok);
    EXPECT_EQ(calls, 2);
    EXPECT_EQ(lastPath, "/other");
}

TEST_F(FollowSeeOtherTest, HeadWith303_StaysHead) {
    int calls{};

    auto next{ [&](Request<HeadMethod, std::string> request) -> Client::ExpResponse<HeadMethod, std::string> {
        ++calls;
        Response<HeadMethod, std::string> r{};
        if (calls == 1) {
            r.status = StatusCodeEnum::SeeOther;
            r.headers.Set("location", "/other");
        } else {
            r.status = StatusCodeEnum::Ok;
        }
        return r;
    } };

    auto pipeline{ FollowSeeOther(next) };
    auto request{ Request<HeadMethod, std::string>::FromUrl("https://example.test/start") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(calls, 2);
    static_assert(std::same_as<decltype(result)::value_type, Response<HeadMethod, std::string>>);
}

TEST_F(FollowSeeOtherTest, NonRedirect_SameMethod_PassesThroughUnchanged) {
    auto next{ [](Request<GetMethod, std::string>) -> Client::ExpResponse<GetMethod, std::string> {
        Response<GetMethod, std::string> r{};
        r.status = StatusCodeEnum::Ok;
        r.body   = "already fine";
        return r;
    } };

    auto pipeline{ FollowSeeOther(next) };
    auto request{ Request<GetMethod, std::string>::FromUrl("https://example.test/x") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(result->body, "already fine");
}

TEST_F(FollowSeeOtherTest, NonRedirect_DifferentMethod_ReturnsError) {
    auto next{ [&]<MethodConcept M>(Request<M, std::string>) -> Client::ExpResponse<M, std::string> {
        Response<M, std::string> r{};
        r.status = StatusCodeEnum::Ok;
        return r;
    } };

    auto pipeline{ FollowSeeOther(next) };
    auto request{ Request<PostMethod, std::string>::FromUrl("https://example.test/x", "body") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_FALSE(result);
}

TEST_F(FollowSeeOtherTest, MissingLocationOn303_ReturnsError) {
    auto next{ [&]<MethodConcept M>(Request<M, std::string>) -> Client::ExpResponse<M, std::string> {
        Response<M, std::string> r{};
        r.status = StatusCodeEnum::SeeOther;
        return r;
    } };

    auto pipeline{ FollowSeeOther(next) };
    auto request{ Request<PostMethod, std::string>::FromUrl("https://example.test/x", "body") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_FALSE(result);
}

TEST_F(FollowSeeOtherTest, TransportErrorOnFirstCall_Propagated) {
    auto next{ [&]<MethodConcept M>(Request<M, std::string>) -> Client::ExpResponse<M, std::string> {
        return std::unexpected{ Thoth::ThothError{ ConnectionErrorEnum::ConnectionFailed } };
    } };

    auto pipeline{ FollowSeeOther(next) };
    auto request{ Request<PostMethod, std::string>::FromUrl("https://example.test/x", "body") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_FALSE(result);
}

TEST_F(FollowSeeOtherTest, ChainedSeeOther_FollowsMultipleHops) {
    int getCalls{};

    auto next{ [&]<MethodConcept M>(Request<M, std::string>) -> Client::ExpResponse<M, std::string> {
        Response<M, std::string> r{};
        if constexpr (std::same_as<M, PostMethod>) {
            r.status = StatusCodeEnum::SeeOther;
            r.headers.Set("location", "/hop1");
        } else {
            ++getCalls;
            if (getCalls < 3) {
                r.status = StatusCodeEnum::SeeOther;
                r.headers.Set("location", "/hop" + std::to_string(getCalls + 1));
            } else {
                r.status = StatusCodeEnum::Ok;
            }
        }
        return r;
    } };

    auto pipeline{ FollowSeeOther(next) };
    auto request{ Request<PostMethod, std::string>::FromUrl("https://example.test/x", "body") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, StatusCodeEnum::Ok);
    EXPECT_EQ(getCalls, 3);
}

TEST_F(FollowSeeOtherTest, ExceedsMaxHops_ReturnsError) {
    auto next{ [&]<MethodConcept M>(Request<M, std::string>) -> Client::ExpResponse<M, std::string> {
        Response<M, std::string> r{};
        r.status = StatusCodeEnum::SeeOther;
        r.headers.Set("location", "/again");
        return r;
    } };

    auto pipeline{ FollowSeeOther(next, /*maxHops=*/2) };
    auto request{ Request<PostMethod, std::string>::FromUrl("https://example.test/x", "body") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_FALSE(result);
}

TEST_F(FollowSeeOtherTest, Downgrade_StripsContentHeaders) {
    Headers seenHeaders;

    auto next{ [&]<MethodConcept M>(Request<M, std::string> request) -> Client::ExpResponse<M, std::string> {
        Response<M, std::string> r{};
        if constexpr (std::same_as<M, PostMethod>) {
            r.status = StatusCodeEnum::SeeOther;
            r.headers.Set("location", "/result");
        } else {
            seenHeaders = request.headers;
            r.status = StatusCodeEnum::Ok;
        }
        return r;
    } };

    auto pipeline{ FollowSeeOther(next) };
    auto request{ Request<PostMethod, std::string>::FromUrl("https://example.test/x", "body") };
    ASSERT_TRUE(request);
    request->headers.Set("content-type", "application/json");

    (void)pipeline(std::move(*request));
    EXPECT_FALSE(seenHeaders.Exists("content-type"));
    EXPECT_FALSE(seenHeaders.Exists("content-length"));
}
#pragma endregion

#pragma region Integration
TEST_F(FollowSeeOtherTest, Integration_HttpBin303_DowngradesToGet) {
    auto request{ Request<PostMethod, std::string>::FromUrl(
        "https://httpbin.org/redirect-to?url=https%3A%2F%2Fhttpbin.org%2Fget&status_code=303",
        "payload"
    ) };
    ASSERT_TRUE(request.has_value());

    auto pipeline{ FollowSeeOther([]<MethodConcept M>(Request<M, std::string> req) {
        return Client::Send(std::move(req), k_networkOptions);
    }) };

    const auto result{ pipeline(std::move(*request)) };
    if (IsUnavailable(result)) GTEST_SKIP() << "httpbin.org is unavailable";

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status, StatusCodeEnum::Ok);
    EXPECT_TRUE(result->body.find("\"https://httpbin.org/get\"") != std::string::npos);
}
#pragma endregion
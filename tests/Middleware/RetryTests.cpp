#include <gtest/gtest.h>
#include <Thoth/Http/Middleware/Retry.hpp>
#include <Thoth/Http/Client/Client.hpp>
#include <Thoth/ThothError.hpp>

#include <chrono>
#include <utility>
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

    RespT MakeResponse(StatusCodeEnum status, Headers headers = {}) {
        RespT r{};
        r.status  = status;
        r.headers = { std::move(headers) };
        return r;
    }
}

#pragma region Mocks
struct RetryTest : testing::Test {
    std::vector<ReqT> calls;

    auto ScriptedNext(std::vector<Client::ExpResponse<GetMethod, std::string>> responses) {
        return [this, responses = std::move(responses), i = std::size_t{}](ReqT request) mutable
            -> Client::ExpResponse<GetMethod, std::string> {
            calls.push_back(request);
            return responses.at(i++);
        };
    }
};

TEST_F(RetryTest, SuccessOnFirstAttempt_NoRetryHappens) {
    auto pipeline{ Retry(3, ScriptedNext({ MakeResponse(StatusCodeEnum::Ok) }), 1ms) };
    auto request{ ReqT::FromUrl("https://example.test/") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(calls.size(), 1u);
}

TEST_F(RetryTest, RetryableTransportError_SucceedsOnSecondAttempt) {
    auto pipeline{ Retry(3, [this, first = true](ReqT request) mutable -> Client::ExpResponse<GetMethod, std::string> {
        calls.push_back(request);
        if (std::exchange(first, false))
            return std::unexpected{ Thoth::ThothError{ ConnectionErrorEnum::ConnectionFailed } };
        return MakeResponse(StatusCodeEnum::Ok);
    }, 1ms) };
    auto request{ ReqT::FromUrl("https://example.test/") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(calls.size(), 2u);
}

TEST_F(RetryTest, Retryable5xxStatus_RetriesAndSucceeds) {
    auto pipeline{ Retry(3, ScriptedNext({
        MakeResponse(StatusCodeEnum::ServiceUnavailable),
        MakeResponse(StatusCodeEnum::Ok)
    }), 1ms) };
    auto request{ ReqT::FromUrl("https://example.test/") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(calls.size(), 2u);
}

TEST_F(RetryTest, NonRetryable4xxStatus_ReturnsImmediately) {
    auto pipeline{ Retry(3, ScriptedNext({ MakeResponse(StatusCodeEnum::NotFound) }), 1ms) };
    auto request{ ReqT::FromUrl("https://example.test/") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, StatusCodeEnum::NotFound);
    EXPECT_EQ(calls.size(), 1u);
}

TEST_F(RetryTest, NonRetryableError_ReturnsImmediately) {
    auto pipeline{ Retry(3, [this](ReqT request) -> Client::ExpResponse<GetMethod, std::string> {
        calls.push_back(request);
        return std::unexpected{ Thoth::ThothError{ Thoth::GenericError{ "not transient" } } };
    }, 1ms) };
    auto request{ ReqT::FromUrl("https://example.test/") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_FALSE(result);
    EXPECT_EQ(calls.size(), 1u);
}

TEST_F(RetryTest, ExhaustsAllAttempts_ReturnsLastFailure) {
    auto pipeline{ Retry(3, [this](ReqT request) -> Client::ExpResponse<GetMethod, std::string> {
        calls.push_back(request);
        return std::unexpected{ Thoth::ThothError{ ConnectionErrorEnum::ConnectionFailed } };
    }, 1ms) };
    auto request{ ReqT::FromUrl("https://example.test/") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_FALSE(result);
    EXPECT_EQ(calls.size(), 3u);
}

TEST_F(RetryTest, ZeroAttempts_StillCallsNextExactlyOnce) {
    auto pipeline{ Retry(0, ScriptedNext({ MakeResponse(StatusCodeEnum::Ok) }), 1ms) };
    auto request{ ReqT::FromUrl("https://example.test/") };
    ASSERT_TRUE(request);

    (void)pipeline(std::move(*request));
    EXPECT_EQ(calls.size(), 1u);
}

TEST_F(RetryTest, RetryAfter_UnderMaxDelay_IsRespected) {
    auto pipeline{ Retry(2, ScriptedNext({
        MakeResponse(StatusCodeEnum::ServiceUnavailable, { { "retry-after", "0" } }),
        MakeResponse(StatusCodeEnum::Ok)
    }), 1ms, true, 1s) };

    auto request{ ReqT::FromUrl("https://example.test/") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(calls.size(), 2u);
}

TEST_F(RetryTest, RetryAfter_ExceedsMaxDelay_AbortsEarly) {
    auto pipeline{ Retry(3, ScriptedNext({
        MakeResponse(StatusCodeEnum::ServiceUnavailable, { { "retry-after", "10" } })
    }), 1ms, true, 1s) };

    auto request{ ReqT::FromUrl("https://example.test/") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(result->status, StatusCodeEnum::ServiceUnavailable);
    EXPECT_EQ(calls.size(), 1u);
}

TEST_F(RetryTest, RetryAfter_Disabled_IgnoresHeader) {
    auto pipeline{ Retry(2, ScriptedNext({
        MakeResponse(StatusCodeEnum::ServiceUnavailable, { { "retry-after", "10" } }),
        MakeResponse(StatusCodeEnum::Ok)
    }), 1ms, false, 1s) };

    auto request{ ReqT::FromUrl("https://example.test/") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(calls.size(), 2u);
}

struct RetryHelperTest : testing::Test {};

TEST_F(RetryHelperTest, IsRetryable_ConnectionError_True) {
    EXPECT_TRUE(Details_::IsRetryable(Thoth::ThothError{ ConnectionErrorEnum::ConnectionFailed }));
}

TEST_F(RetryHelperTest, IsRetryable_MessageParseError_True) {
    EXPECT_TRUE(Details_::IsRetryable(Thoth::ThothError{ MessageParseErrorEnum::InvalidStartLine }));
}

TEST_F(RetryHelperTest, IsRetryable_GenericError_False) {
    EXPECT_FALSE(Details_::IsRetryable(Thoth::ThothError{ Thoth::GenericError{ "x" } }));
}

TEST_F(RetryHelperTest, IsRetryable_Status_500_True) {
    EXPECT_TRUE(Details_::IsRetryable(StatusCodeEnum::InternalServerError));
}

TEST_F(RetryHelperTest, IsRetryable_Status_404_False) {
    EXPECT_FALSE(Details_::IsRetryable(StatusCodeEnum::NotFound));
}

TEST_F(RetryHelperTest, RetryBackoff_DoublesEachAttempt) {
    EXPECT_EQ(Details_::RetryBackoff(0, 100ms), 100ms);
    EXPECT_EQ(Details_::RetryBackoff(1, 100ms), 200ms);
    EXPECT_EQ(Details_::RetryBackoff(2, 100ms), 400ms);
}

TEST_F(RetryHelperTest, RetryBackoff_CapsGrowth_NoOverflow) {
    EXPECT_NO_FATAL_FAILURE({ auto d{ Details_::RetryBackoff(1000, 100ms) }; (void)d; });
}
#pragma endregion

#pragma region Integration
TEST_F(RetryTest, Integration_HttpBin503_ExhaustsRetriesAndReturns503) {
    auto request{ ReqT::FromUrl("https://httpbingo.org/status/503") };
    ASSERT_TRUE(request.has_value());

    int calls{};
    auto pipeline{ Retry(3, [&calls](ReqT req) {
        ++calls;
        return Client::Send(std::move(req), k_networkOptions);
    }, 1ms) };

    const auto result{ pipeline(std::move(*request)) };
    if (IsUnavailable(result)) GTEST_SKIP() << "httpbingo.org is unavailable";

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status, StatusCodeEnum::ServiceUnavailable);
    EXPECT_EQ(calls, 3);
}
#pragma endregion
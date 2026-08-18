#include <gtest/gtest.h>

#include <Hermes/Utils/Overloads.hpp>
#include <Thoth/Http/Client/Client.hpp>
#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/_base/Http1.hpp>
#include <Thoth/Utils/Ranges/SharedInputView.hpp>

#include <chrono>
#include <string>
#include <string_view>

#include <Thoth/ThothError.hpp>

using namespace std::chrono_literals;
using namespace Thoth::Http;

namespace {
    constexpr std::string_view k_exampleUrl{ "http://www.example.com/" };
    constexpr std::string_view k_slowResponseUrl{ "https://httpbin.org/delay/3" };

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

        const auto currErr{ result.error().template As<ConnectionErrorEnum>() };

        return std::ranges::contains(k_values, currErr);
    }
}

struct ClientTest : testing::Test {};

TEST_F(ClientTest, Send_ExampleCom_DefaultRequestTimeout_Succeeds) {
    auto request{ GetRequest::FromUrl(k_exampleUrl) };
    ASSERT_TRUE(request.has_value());

    const auto result{ Client::Send(std::move(*request), k_networkOptions) };
    if (IsUnavailable(result))
        GTEST_SKIP() << "www.example.com is unavailable";

    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->body.empty());
}

TEST_F(ClientTest, Send_ExampleCom_ExpiredRequestTimeout_ReturnsSendTimeout) {
    auto request{ GetRequest::FromUrl(k_exampleUrl) };
    ASSERT_TRUE(request.has_value());

    auto options{ k_networkOptions };
    options.requestTimeout = 0ms;

    const auto result{ Client::Send(std::move(*request), options) };
    if (IsUnavailable(result))
        GTEST_SKIP() << "www.example.com is unavailable";

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), ConnectionErrorEnum::SendTimeout);
}

TEST_F(ClientTest, Httpbin_DelayedResponse_RequestTimeout_ReturnsParseError) {
    auto request{ GetRequest::FromUrl(k_slowResponseUrl) };
    ASSERT_TRUE(request.has_value());

    auto options{ k_networkOptions };
    options.requestTimeout = 100ms;

    const auto result{ Client::Send(std::move(*request), options) };
    if (IsUnavailable(result))
        GTEST_SKIP() << "httpbin.org is unavailable";

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), MessageParseErrorEnum::InvalidStartLine);
}

TEST_F(ClientTest, Request_ExplicitPort_IsPreservedInAuthority) {
    const auto request{ GetRequest::FromUrl("http://localhost:8080/") };
    ASSERT_TRUE(request);

    const auto authority{ request->url.GetAuthority() };
    ASSERT_TRUE(authority);
    ASSERT_TRUE(authority->port);
    EXPECT_EQ(*authority->port, 8080u);
}

TEST_F(ClientTest, PrepareBodyHeaders_SizedBody_UsesContentLength) {
    Headers headers{
        { "content-length",    "999"    },
        { "transfer-encoding", "chunked" }
    };

    details_::Http1::PrepareBodyHeaders(headers, std::string{ "body" });

    EXPECT_TRUE(headers.Exists("content-length", "4"));
    EXPECT_FALSE(headers.Exists("transfer-encoding"));
}


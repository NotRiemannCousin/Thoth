#include <gtest/gtest.h>
#include <Thoth/Http/Middleware/Decompress.hpp>
#include <Thoth/Http/Client/Client.hpp>
#include <Thoth/ThothError.hpp>

#include <chrono>

using namespace Thoth::Http;
using namespace std::chrono_literals;

namespace {
    using ReqT  = Request<GetMethod, std::string>;
    using RespT = Response<GetMethod, std::string>;

    const std::string k_plain{ "Thoth middleware test" };
    const std::string k_gzip{ "\x1f\x8b\x08\x00\x00\x00\x00\x00\x02\xff\x0b\xc9\xc8\x2f\xc9\x50\xc8\xcd\x4c\x49\xc9\x49\x2d\x4f\x2c\x4a\x55\x28\x49\x2d\x2e\x01\x00\xe7\x20\x2c\xb1\x15\x00\x00\x00", 41 };
    const std::string k_zlibDeflate{ "\x78\x9c\x0b\xc9\xc8\x2f\xc9\x50\xc8\xcd\x4c\x49\xc9\x49\x2d\x4f\x2c\x4a\x55\x28\x49\x2d\x2e\x01\x00\x58\x8d\x08\x26", 29 };

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

    RespT MakeResponse(std::string body, Headers headers = {}) {
        RespT r{};
        r.status  = StatusCodeEnum::Ok;
        r.headers = { std::move(headers) };
        r.body    = std::move(body);
        return r;
    }
}

#pragma region Mocks
struct DecompressTest : testing::Test {
    auto ScriptedNext(RespT response) {
        return [response = std::move(response)](ReqT) -> Client::ExpResponse<GetMethod, std::string> {
            return response;
        };
    }
};

TEST_F(DecompressTest, NoContentEncoding_PassesThroughUnchanged) {
    auto pipeline{ Decompress(ScriptedNext(MakeResponse(k_plain))) };
    auto request{ ReqT::FromUrl("https://example.test/") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(result->body, k_plain);
}

TEST_F(DecompressTest, GzipEncoding_DecompressesToOriginalBody) {
    auto pipeline{ Decompress(ScriptedNext(MakeResponse(k_gzip, { { "content-encoding", "gzip" } }))) };
    auto request{ ReqT::FromUrl("https://example.test/") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(result->body, k_plain);
}

TEST_F(DecompressTest, DeflateEncoding_DecompressesToOriginalBody) {
    auto pipeline{ Decompress(ScriptedNext(MakeResponse(k_zlibDeflate, { { "content-encoding", "deflate" } }))) };
    auto request{ ReqT::FromUrl("https://example.test/") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_EQ(result->body, k_plain);
}

TEST_F(DecompressTest, GzipEncoding_RemovesContentEncodingHeader) {
    auto pipeline{ Decompress(ScriptedNext(MakeResponse(k_gzip, { { "content-encoding", "gzip" } }))) };
    auto request{ ReqT::FromUrl("https://example.test/") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_FALSE(result->headers.Exists("content-encoding"));
}

TEST_F(DecompressTest, GzipEncoding_RemovesStaleContentLength) {
    auto pipeline{ Decompress(ScriptedNext(MakeResponse(k_gzip,
        { { "content-encoding", "gzip" }, { "content-length", "41" } }))) };
    auto request{ ReqT::FromUrl("https://example.test/") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_TRUE(result);
    EXPECT_FALSE(result->headers.Exists("content-length"));
}

TEST_F(DecompressTest, UnsupportedEncoding_ReturnsError) {
    auto pipeline{ Decompress(ScriptedNext(MakeResponse("whatever", { { "content-encoding", "br" } }))) };
    auto request{ ReqT::FromUrl("https://example.test/") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_FALSE(result);
}

TEST_F(DecompressTest, TransportError_PropagatedUnchanged) {
    auto pipeline{ Decompress([](ReqT) -> Client::ExpResponse<GetMethod, std::string> {
        return std::unexpected{ Thoth::ThothError{ ConnectionErrorEnum::ConnectionFailed } };
    }) };
    auto request{ ReqT::FromUrl("https://example.test/") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_FALSE(result);
}

TEST_F(DecompressTest, MalformedGzipBody_ReturnsError) {
    auto pipeline{ Decompress(ScriptedNext(MakeResponse("not actually gzip", { { "content-encoding", "gzip" } }))) };
    auto request{ ReqT::FromUrl("https://example.test/") };
    ASSERT_TRUE(request);

    const auto result{ pipeline(std::move(*request)) };
    ASSERT_FALSE(result);
}

struct InflateHelperTest : testing::Test {};

TEST_F(InflateHelperTest, InflateGzipOrDeflate_GzipBytes_ReturnsPlainText) {
    const auto result{ details_::RangeInflateGzipOrDeflate(k_gzip) };
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, k_plain);
}

TEST_F(InflateHelperTest, InflateGzipOrDeflate_ZlibBytes_ReturnsPlainText) {
    const auto result{ details_::RangeInflateGzipOrDeflate(k_zlibDeflate) };
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, k_plain);
}

TEST_F(InflateHelperTest, InflateGzipOrDeflate_GarbageInput_ReturnsError) {
    const auto result{ details_::RangeInflateGzipOrDeflate("definitely not compressed") };
    EXPECT_FALSE(result);
}
#pragma endregion

#pragma region Integration
TEST_F(DecompressTest, Integration_HttpBinGzip_SucceedsAndDecodes) {
    auto request{ ReqT::FromUrl("https://httpbin.org/gzip") };
    ASSERT_TRUE(request.has_value());

    auto pipeline{ Decompress([](ReqT req) { return Client::Send(std::move(req), k_networkOptions); }) };

    const auto result{ pipeline(std::move(*request)) };
    if (IsUnavailable(result)) GTEST_SKIP() << "httpbin.org is unavailable";

    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->status, StatusCodeEnum::Ok);
    EXPECT_TRUE(result->body.find("\"gzipped\": true") != std::string::npos);
    EXPECT_FALSE(result->headers.Exists("content-encoding"));
}
#pragma endregion
#include <gtest/gtest.h>
#include <Thoth/Http/NHeaders/Request/RequestHeaders.hpp>
#include <Thoth/Http/NHeaders/Response/ResponseHeaders.hpp>

#include <string>

using namespace Thoth::Http;


#pragma region RequestHeaders - Construction & Base

struct RequestHeadersTest : testing::Test {
    RequestHeaders h{{
        { "host",          "example.com"      },
        { "authorization", "Bearer tok123"    },
        { "content-type",  "application/json" },
        { "referrer",      "https://ref.com"  },
        { "from",          "user@example.com" }
    }};
};

TEST_F(RequestHeadersTest, InheritsHeaders_Exists_CaseInsensitive) {
    EXPECT_TRUE(h.Exists("HOST"));
    EXPECT_TRUE(h.Exists("Content-Type"));
}

TEST_F(RequestHeadersTest, InheritsHeaders_Get_ReturnsValue) {
    const auto r{ h.Get("host") };
    ASSERT_TRUE(r);
    EXPECT_EQ(**r, "example.com");
}

TEST_F(RequestHeadersTest, InheritsHeaders_Add_Succeeds) {
    RequestHeaders tmp{ h };
    tmp.Add("x-custom", "value");
    EXPECT_TRUE(tmp.Exists("x-custom", "value"));
}

TEST_F(RequestHeadersTest, InheritsHeaders_Remove_Succeeds) {
    RequestHeaders tmp{ h };
    EXPECT_TRUE(tmp.Remove("from", "user@example.com"));
    EXPECT_FALSE(tmp.Exists("from"));
}

TEST_F(RequestHeadersTest, InheritsHeaders_Size_Correct) {
    EXPECT_EQ(h.Size(), 5u);
}

TEST_F(RequestHeadersTest, InheritsHeaders_Clear_EmptiesAll) {
    RequestHeaders tmp{ h };
    tmp.Clear();
    EXPECT_TRUE(tmp.Empty());
}

#pragma endregion


#pragma region RequestHeaders - Host Proxy

struct RequestHostProxyTest : testing::Test {
    RequestHeaders h{{ { "host", "example.com" } }};
};

TEST_F(RequestHostProxyTest, Host_Get_ReturnsValue) {
    const auto proxy{ h.Host().Get() };
    ASSERT_TRUE(proxy);
    EXPECT_EQ(*proxy, "example.com");
}

TEST_F(RequestHostProxyTest, Host_Const_Get_ReturnsValue) {
    const RequestHeaders& ch{ h };
    const auto proxy{ ch.Host().Get() };
    ASSERT_TRUE(proxy);
    EXPECT_EQ(*proxy, "example.com");
}

TEST_F(RequestHostProxyTest, Host_Set_UpdatesHeader) {
    h.Host().Set("other.com");
    EXPECT_TRUE(h.Exists("host", "other.com"));
}

TEST_F(RequestHostProxyTest, Host_Missing_GetReturnsNullopt) {
    RequestHeaders empty{};
    EXPECT_FALSE(empty.Host().Get());
}

#pragma endregion


#pragma region RequestHeaders - Authorization Proxy

struct RequestAuthProxyTest : testing::Test {
    RequestHeaders h{{ { "authorization", "Bearer tok123" } }};
};

TEST_F(RequestAuthProxyTest, Authorization_Get_ReturnsValue) {
    const auto proxy{ h.Authorization().Get() };
    ASSERT_TRUE(proxy);
    EXPECT_EQ(*proxy, "Bearer tok123");
}

TEST_F(RequestAuthProxyTest, Authorization_Set_UpdatesHeader) {
    h.Authorization().Set("Basic dXNlcjpwYXNz");
    EXPECT_TRUE(h.Exists("authorization", "Basic dXNlcjpwYXNz"));
}

TEST_F(RequestAuthProxyTest, Authorization_Missing_ReturnsFalse) {
    RequestHeaders empty{};
    EXPECT_FALSE(empty.Authorization().Get());
}

#pragma endregion


#pragma region RequestHeaders - Referrer Proxy

struct RequestReferrerProxyTest : testing::Test {
    RequestHeaders h{{ { "referrer", "https://origin.com" } }};
};

TEST_F(RequestReferrerProxyTest, Referrer_Get_ReturnsValue) {
    EXPECT_TRUE(h.Referrer().Get());
    EXPECT_EQ(*h.Referrer().Get(), "https://origin.com");
}

TEST_F(RequestReferrerProxyTest, Referrer_Set_UpdatesHeader) {
    h.Referrer().Set("https://new.com");
    EXPECT_TRUE(h.Exists("referrer", "https://new.com"));
}

#pragma endregion


#pragma region RequestHeaders - From Proxy

struct RequestFromProxyTest : testing::Test {
    RequestHeaders h{{ { "from", "admin@example.com" } }};
};

TEST_F(RequestFromProxyTest, From_Get_ReturnsValue) {
    EXPECT_TRUE(h.From().Get());
    EXPECT_EQ(*h.From().Get(), "admin@example.com");
}

TEST_F(RequestFromProxyTest, From_Set_UpdatesHeader) {
    h.From().Set("other@example.com");
    EXPECT_TRUE(h.Exists("from", "other@example.com"));
}

#pragma endregion


#pragma region ResponseHeaders - Construction & Base

struct ResponseHeadersTest : testing::Test {
    ResponseHeaders h{{
        { "server",        "nginx/1.18"       },
        { "location",      "https://new.com"  },
        { "content-type",  "text/html"        },
        { "vary",          "Accept-Encoding"  }
    }};
};

TEST_F(ResponseHeadersTest, InheritsHeaders_Exists_CaseInsensitive) {
    EXPECT_TRUE(h.Exists("SERVER"));
    EXPECT_TRUE(h.Exists("content-type"));
}

TEST_F(ResponseHeadersTest, InheritsHeaders_Get_ReturnsValue) {
    const auto r{ h.Get("location") };
    ASSERT_TRUE(r);
    EXPECT_EQ(**r, "https://new.com");
}

TEST_F(ResponseHeadersTest, InheritsHeaders_Add_Succeeds) {
    ResponseHeaders tmp{ h };
    tmp.Add("x-powered-by", "Thoth");
    EXPECT_TRUE(tmp.Exists("x-powered-by", "Thoth"));
}

TEST_F(ResponseHeadersTest, InheritsHeaders_Size_Correct) {
    EXPECT_EQ(h.Size(), 4u);
}

#pragma endregion


#pragma region ResponseHeaders - Server Proxy

struct ResponseServerProxyTest : testing::Test {
    ResponseHeaders h{{ { "server", "nginx/1.18" } }};
};

TEST_F(ResponseServerProxyTest, Server_Get_ReturnsValue) {
    EXPECT_TRUE(h.Server().Get());
    EXPECT_EQ(*h.Server().Get(), "nginx/1.18");
}

TEST_F(ResponseServerProxyTest, Server_Const_Get_ReturnsValue) {
    const ResponseHeaders& ch{ h };
    EXPECT_TRUE(ch.Server().Get());
}

TEST_F(ResponseServerProxyTest, Server_Set_UpdatesHeader) {
    h.Server().Set("Thoth/0.1");
    EXPECT_TRUE(h.Exists("server", "Thoth/0.1"));
}

TEST_F(ResponseServerProxyTest, Server_Missing_ReturnsFalse) {
    ResponseHeaders empty{};
    EXPECT_FALSE(empty.Server().Get());
}

#pragma endregion


#pragma region ResponseHeaders - Location Proxy

struct ResponseLocationProxyTest : testing::Test {
    ResponseHeaders h{{ { "location", "https://redirect.com" } }};
};

TEST_F(ResponseLocationProxyTest, Location_Get_ReturnsValue) {
    EXPECT_TRUE(h.Location().Get());
    EXPECT_EQ(*h.Location().Get(), "https://redirect.com");
}

TEST_F(ResponseLocationProxyTest, Location_Set_UpdatesHeader) {
    h.Location().Set("https://other.com");
    EXPECT_TRUE(h.Exists("location", "https://other.com"));
}

TEST_F(ResponseLocationProxyTest, Location_Missing_ReturnsFalse) {
    ResponseHeaders empty{};
    EXPECT_FALSE(empty.Location().Get());
}

#pragma endregion


#pragma region ResponseHeaders - ProxyAuthenticate Proxy

struct ResponseProxyAuthTest : testing::Test {
    ResponseHeaders h{{ { "proxy-authenticate", R"(Basic realm="Proxy")" } }};
};

TEST_F(ResponseProxyAuthTest, ProxyAuthenticate_Get_ReturnsValue) {
    EXPECT_TRUE(h.ProxyAuthenticate().Get());
}

TEST_F(ResponseProxyAuthTest, ProxyAuthenticate_Set_UpdatesHeader) {
    h.ProxyAuthenticate().Set("Bearer");
    EXPECT_TRUE(h.Exists("proxy-authenticate", "Bearer"));
}

#pragma endregion

#pragma region Headers - Content-Type (ValueProxy<MimeType>)

struct ContentTypeProxyTest : testing::Test {
    Headers h{ { "content-type", R"(application/json; charset=utf-8; boundary="something")" } };
};

TEST_F(ContentTypeProxyTest, Get_ReturnsParsedMimeType) {
    const auto res{ h.ContentType().Get() };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->type, "application");
    EXPECT_EQ(res->subtype, "json");
    ASSERT_EQ(res->options.size(), 2);
    EXPECT_EQ(res->options[0].first, "charset");
    EXPECT_EQ(res->options[0].second, "utf-8");
    EXPECT_EQ(res->options[1].first, "boundary");
    EXPECT_EQ(res->options[1].second, "something");
}

TEST_F(ContentTypeProxyTest, Set_FormatsMimeTypeCorrectly) {
    Headers tmp{};

    NHeaders::MimeType mime{ "text", "html", {{"charset", "utf-8"}} };
    tmp.ContentType().Set(mime);

    EXPECT_TRUE(tmp.Exists("content-type", "text/html;charset=utf-8"));
}

#pragma endregion


#pragma region Headers - Accept-Encoding (ListProxy<Enum>)

struct AcceptEncodingProxyTest : testing::Test {
    Headers h{ { "accept-encoding", "gzip, deflate, br, *" } };
};

TEST_F(AcceptEncodingProxyTest, Get_ReturnsParsedEnums) {
    using NHeaders::AcceptEncodingEnum;

    const auto res{ h.AcceptEncoding().Get() };
    ASSERT_TRUE(res);
    ASSERT_EQ(res->size(), 4);
    EXPECT_EQ(res->at(0), AcceptEncodingEnum::Gzip);
    EXPECT_EQ(res->at(1), AcceptEncodingEnum::Deflate);
    EXPECT_EQ(res->at(2), AcceptEncodingEnum::Br);
    EXPECT_EQ(res->at(3), AcceptEncodingEnum::Identity); // * mapped to Identity
}

TEST_F(AcceptEncodingProxyTest, Set_FormatsEnumsCorrectly) {
    using NHeaders::AcceptEncodingEnum;

    Headers tmp{};
    std::vector encodings{ AcceptEncodingEnum::Zstd, AcceptEncodingEnum::Dcb };
    tmp.AcceptEncoding().Set(encodings);

    EXPECT_TRUE(tmp.Exists("accept-encoding", "zstd,dcb"));
}

#pragma endregion


#pragma region Headers - Content-Length (ValueProxy<Numeric>)

struct ContentLengthProxyTest : testing::Test {
    Headers h{ { "content-length", "1048576" } };
};

TEST_F(ContentLengthProxyTest, Get_ReturnsParsedNumber) {
    const auto res{ h.ContentLength().Get() };
    ASSERT_TRUE(res);
    EXPECT_EQ(*res, 1048576u);
}

TEST_F(ContentLengthProxyTest, Set_FormatsNumberCorrectly) {
    Headers tmp{};
    tmp.ContentLength().Set(42);
    EXPECT_TRUE(tmp.Exists("content-length", "42"));
}

#pragma endregion


#pragma region Headers - Upgrade (ListProxy<Upgrade>)

struct UpgradeProxyTest : testing::Test {
    using Upgrade = NHeaders::Upgrade;

    Headers h{ { "upgrade", "HTTP/2.0, websocket, IRC/6.9" } };
};

TEST_F(UpgradeProxyTest, Get_ReturnsParsedUpgrades) {
    const auto res{ h.Upgrade().Get() };
    ASSERT_TRUE(res);
    ASSERT_EQ(res->size(), 3);

    EXPECT_EQ(res->at(0), (Upgrade{ "HTTP"     , { "2.0" }    }));
    EXPECT_EQ(res->at(1), (Upgrade{ "websocket", std::nullopt }));
    EXPECT_EQ(res->at(2), (Upgrade{ "IRC"      , { "6.9" }    }));
}

#pragma endregion


#pragma region ResponseHeaders - EntityTag (ValueProxy<EntityTag>)

struct EntityTagProxyTest : testing::Test {
    ResponseHeaders h{{ { "etag", R"(W/"0815")" } }};
};

TEST_F(EntityTagProxyTest, Get_ReturnsParsedWeakTag) {
    const auto res{ h.EntityTag().Get() };
    ASSERT_TRUE(res);
    EXPECT_TRUE(res->isWeak);
    EXPECT_EQ(res->tag, "0815");
}

TEST_F(EntityTagProxyTest, Get_ReturnsParsedStrongTag) {
    ResponseHeaders strong{{ { "etag", R"("xyzzy")" } }};
    const auto res{ strong.EntityTag().Get() };
    ASSERT_TRUE(res);
    EXPECT_FALSE(res->isWeak);
    EXPECT_EQ(res->tag, "xyzzy");
}

TEST_F(EntityTagProxyTest, Set_FormatsTagCorrectly) {
    ResponseHeaders tmp{};
    tmp.EntityTag().Set(NHeaders::EntityTag{ "my-tag", true });
    EXPECT_TRUE(tmp.Exists("etag", R"(W/"my-tag")"));
}

#pragma endregion


#pragma region ResponseHeaders - AcceptRanges (ValueProxy<Enum>)

struct AcceptRangesProxyTest : testing::Test {
    ResponseHeaders h{{ { "accept-ranges", "bytes" } }};
};

TEST_F(AcceptRangesProxyTest, Get_ReturnsParsedRangeEnum) {
    const auto res{ h.AcceptRanges().Get() };
    ASSERT_TRUE(res);
    EXPECT_EQ(*res, NHeaders::AcceptRanges::Bytes);
}

TEST_F(AcceptRangesProxyTest, Set_FormatsRangeEnumCorrectly) {
    ResponseHeaders tmp{};
    tmp.AcceptRanges().Set(NHeaders::AcceptRanges::None);
    EXPECT_TRUE(tmp.Exists("accept-ranges", "none"));
}

#pragma endregion


#pragma region ResponseHeaders - Age (ValueProxy<chrono>)

struct AgeProxyTest : testing::Test {
    ResponseHeaders h{{ { "age", "86400" } }};
};

TEST_F(AgeProxyTest, Get_ReturnsParsedSeconds) {
    const auto res{ h.Age().Get() };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->count(), 86400);
}

#pragma endregion

#pragma region Headers - Range (Scanner & Formatter)

// Testando o Scanner diretamente, pois as regras de negócio do Range são complexas (Prefix/Suffix)
struct RangeScannerTest : testing::Test {
    using PrefixedRange = NHeaders::PrefixedRange;
    using SuffixedRange = NHeaders::SuffixedRange;
    using Range = NHeaders::Range;

    Thoth::Utils::Scanner<NHeaders::Range> scanner{};
};

TEST_F(RangeScannerTest, Scan_PrefixedWithEnd) {
    const auto res{ scanner.Scan("bytes=200-1000") };
    ASSERT_TRUE(res);
    ASSERT_TRUE(std::holds_alternative<PrefixedRange>(*res));

    const auto& p{ std::get<PrefixedRange>(*res) };
    EXPECT_EQ(p.start, 200u);
    ASSERT_TRUE(p.end);
    EXPECT_EQ(*p.end, 1000u);
}

TEST_F(RangeScannerTest, Scan_PrefixedWithoutEnd) {
    const auto res{ scanner.Scan("bytes=500-") };
    ASSERT_TRUE(res);
    ASSERT_TRUE(std::holds_alternative<PrefixedRange>(*res));

    const auto& p{ std::get<PrefixedRange>(*res) };
    EXPECT_EQ(p.start, 500u);
    EXPECT_FALSE(p.end);
}

TEST_F(RangeScannerTest, Scan_Suffixed) {
    const auto res{ scanner.Scan("bytes=-42") };
    ASSERT_TRUE(res);
    ASSERT_TRUE(std::holds_alternative<SuffixedRange>(*res));
    EXPECT_EQ(std::get<SuffixedRange>(*res).last, 42u);
}

TEST_F(RangeScannerTest, Scan_InvalidReturnsNullopt) {
    EXPECT_FALSE(scanner.Scan("chars=0-10"));
    EXPECT_FALSE(scanner.Scan("bytes=abc"));
    EXPECT_FALSE(scanner.Scan("bytes=100-20-30"));
    EXPECT_FALSE(scanner.Scan(""));
}

TEST_F(RangeScannerTest, Formatter_FormatsCorrectly) {
    constexpr Range prefFull{ PrefixedRange{ 10, 50 } };
    constexpr Range prefOpen{ PrefixedRange{ 100, std::nullopt } };
    constexpr Range suff{ SuffixedRange{ 500 } };

    EXPECT_EQ(std::format("{}", prefFull), "bytes=10-50");
    EXPECT_EQ(std::format("{}", prefOpen), "bytes=100-");
    EXPECT_EQ(std::format("{}", suff),     "bytes=-500");
}

#pragma endregion


#pragma region ResponseHeaders - RetryAfter (ValueProxy Multiple Types)

struct RetryAfterProxyTest : testing::Test {
    ResponseHeaders h{{ { "retry-after", "120" } }};
};

TEST_F(RetryAfterProxyTest, Get_ReturnsSecondsVariant) {
    auto res{ h.RetryAfter().Get() };
    ASSERT_TRUE(res);
    ASSERT_TRUE(std::holds_alternative<std::chrono::seconds>(*res));
    EXPECT_EQ(std::get<std::chrono::seconds>(*res).count(), 120);
}

TEST_F(RetryAfterProxyTest, GetWithDefault_FallsBackOnMissing) {
    ResponseHeaders empty{};
    const std::variant<std::chrono::utc_clock::time_point, std::chrono::seconds> def{ std::chrono::seconds{30} };

    auto res{ empty.RetryAfter().GetWithDefault(def) };
    ASSERT_TRUE(res);
    ASSERT_TRUE(std::holds_alternative<std::chrono::seconds>(*res));
    EXPECT_EQ(std::get<std::chrono::seconds>(*res).count(), 30);
}

#pragma endregion


#pragma region Proxy Error Handling (List & Value)

struct ProxyErrorTest : testing::Test {
    using HeaderErrorEnum = NHeaders::HeaderErrorEnum;

    Headers h{
        { "content-length" , "not-a-number" },
        { "accept-encoding", ""             }
    };
};

TEST_F(ProxyErrorTest, ValueProxy_Get_ReturnsNotFound) {
    Headers h0{
        { "content-length", "not-a-number" },
        { "accept-encoding", ""            }
    };


    auto missing{ h.ContentLocation().Get() };
    EXPECT_FALSE(missing);
    EXPECT_EQ(missing.error(), HeaderErrorEnum::NotFound);
}

TEST_F(ProxyErrorTest, ValueProxy_Get_ReturnsInvalidFormat) {
    auto invalid{ h.ContentLength().Get() };
    EXPECT_FALSE(invalid);
    EXPECT_EQ(invalid.error(), HeaderErrorEnum::InvalidFormat);
}

TEST_F(ProxyErrorTest, ListProxy_Get_ReturnsEmptyValue) {
    auto emptyVal{ h.AcceptEncoding().Get() };
    EXPECT_FALSE(emptyVal);
    EXPECT_EQ(emptyVal.error(), HeaderErrorEnum::EmptyValue);
}

TEST_F(ProxyErrorTest, ListProxy_GetWithDefault_ReturnsDefaultOnMissing) {
    Headers empty{};
    const std::vector<NHeaders::MimeType> def{{ "text", "plain" }};

    auto res{ empty.Accept().GetWithDefault(def) };
    ASSERT_TRUE(res);
    ASSERT_EQ(res->size(), 1);
    EXPECT_EQ(res->at(0).type, "text");
}

TEST_F(ProxyErrorTest, ListProxy_TrySet_FailsOnInvalidInput) {
    Headers empty{};
    EXPECT_FALSE(empty.AcceptEncoding().TrySet("not_an_enum_value"));
    EXPECT_TRUE(empty.Empty());
}

#pragma endregion
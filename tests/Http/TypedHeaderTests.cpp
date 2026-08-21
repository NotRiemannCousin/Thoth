// ReSharper disable CppUseDesignatedInitializers
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
    RequestHeaders h{{ { "referer", "https://origin.com" } }};
};

TEST_F(RequestReferrerProxyTest, Referrer_Get_ReturnsValue) {
    EXPECT_TRUE(h.Referrer().Get());
    EXPECT_EQ(*h.Referrer().Get(), "https://origin.com");
}

TEST_F(RequestReferrerProxyTest, Referrer_Set_UpdatesHeader) {
    h.Referrer().Set("https://new.com");
    EXPECT_TRUE(h.Exists("referer", "https://new.com"));
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

TEST_F(ResponseProxyAuthTest, ProxyAuthenticate_Get_ReturnsParsedValue) {
    const auto proxy{ h.ProxyAuthenticate().Get() };
    ASSERT_TRUE(proxy);
    ASSERT_EQ(proxy->size(), 1u);
    EXPECT_EQ(proxy->at(0).scheme, "Basic");
    ASSERT_EQ(proxy->at(0).params.size(), 1u);
    EXPECT_EQ(proxy->at(0).params[0].first, "realm");
    EXPECT_EQ(proxy->at(0).params[0].second, "Proxy");
}

TEST_F(ResponseProxyAuthTest, ProxyAuthenticate_Set_UpdatesHeader) {
    std::vector<NHeaders::Challenge> challenges{ {"Bearer", {}} };
    h.ProxyAuthenticate().Set(challenges);
    EXPECT_TRUE(h.Exists("proxy-authenticate", "Bearer"));
}

TEST_F(ResponseProxyAuthTest, ProxyAuthenticate_TrySet_UpdatesHeader) {
    EXPECT_TRUE(h.ProxyAuthenticate().TrySet("Bearer"));
    EXPECT_TRUE(h.Exists("proxy-authenticate", "Bearer"));
}

TEST_F(ResponseProxyAuthTest, ProxyAuthenticate_Add_AppendsChallenge) {
    h.ProxyAuthenticate().Add(NHeaders::Challenge{"Digest", {{"qop", "auth"}}});
    EXPECT_TRUE(h.Exists("proxy-authenticate", R"(Basic realm="Proxy")"));
    EXPECT_TRUE(h.Exists("proxy-authenticate", R"(Digest qop="auth")"));
}

#pragma endregion


#pragma region ResponseHeaders - WwwAuthenticate Proxy

struct ResponseWwwAuthTest : testing::Test {
    ResponseHeaders h{{
        { "www-authenticate", R"(Newauth realm="apps", type="1")" },
        { "www-authenticate", R"(Basic realm="simple")" }
    }};
};

TEST_F(ResponseWwwAuthTest, WwwAuthenticate_Get_ReturnsParsedValues) {
    const auto res{ h.WwwAuthenticate().Get() };
    ASSERT_TRUE(res);
    ASSERT_EQ(res->size(), 2u);

    EXPECT_EQ(res->at(0).scheme, "Newauth");
    ASSERT_EQ(res->at(0).params.size(), 2u);
    EXPECT_EQ(res->at(0).params[0].first, "realm");
    EXPECT_EQ(res->at(0).params[0].second, "apps");
    EXPECT_EQ(res->at(0).params[1].first, "type");
    EXPECT_EQ(res->at(0).params[1].second, "1");

    EXPECT_EQ(res->at(1).scheme, "Basic");
    ASSERT_EQ(res->at(1).params.size(), 1u);
    EXPECT_EQ(res->at(1).params[0].first, "realm");
    EXPECT_EQ(res->at(1).params[0].second, "simple");
}

#pragma endregion


#pragma region ResponseHeaders - SetCookie Proxy

struct ResponseSetCookieTest : testing::Test {
    ResponseHeaders h{{
        { "set-cookie", "session=abc; Path=/; Secure; HttpOnly" },
        { "set-cookie", "theme=dark; SameSite=Lax" }
    }};
};

TEST_F(ResponseSetCookieTest, SetCookie_Get_ReturnsParsedCookies) {
    const auto res{ h.SetCookie().Get() };
    ASSERT_TRUE(res);
    ASSERT_EQ(res->size(), 2u);

    EXPECT_EQ(res->at(0).name, "session");
    EXPECT_EQ(res->at(0).value, "abc");
    EXPECT_TRUE(res->at(0).path);
    EXPECT_EQ(*res->at(0).path, "/");
    EXPECT_TRUE(res->at(0).secure);
    EXPECT_TRUE(res->at(0).httpOnly);

    EXPECT_EQ(res->at(1).name, "theme");
    EXPECT_EQ(res->at(1).value, "dark");
    EXPECT_TRUE(res->at(1).sameSite);
    EXPECT_EQ(*res->at(1).sameSite, NHeaders::SameSiteEnum::Lax);
}

TEST_F(ResponseSetCookieTest, SetCookie_Add_IncreasesCount) {
    NHeaders::Cookie c{ .name = "new", .value = "val", .maxAge = 3600 };
    h.SetCookie().Add(c);

    const auto res{ h.SetCookie().Get() };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->size(), 3u);
    EXPECT_EQ(res->at(2).name, "new");
    EXPECT_EQ(res->at(2).maxAge, 3600);
}

TEST_F(ResponseSetCookieTest, SetCookie_TrySet_ReplacesValues) {
    EXPECT_TRUE(h.SetCookie().TrySet("a=1"));

    const auto res{ h.SetCookie().Get() };
    ASSERT_TRUE(res);
    ASSERT_EQ(res->size(), 1u);
    EXPECT_EQ(res->at(0).name, "a");
    EXPECT_EQ(res->at(0).value, "1");
}

#pragma endregion


#pragma region Headers - Challenge (Scanner & Formatter)

struct ChallengeScannerTest : testing::Test {
    Thoth::Utils::Scanner<NHeaders::Challenge> scanner{};
};

TEST_F(ChallengeScannerTest, Scan_BareScheme) {
    const auto res{ scanner.Scan("Bearer") };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->scheme, "Bearer");
    EXPECT_TRUE(res->params.empty());
}

TEST_F(ChallengeScannerTest, Scan_Token68) {
    const auto res{ scanner.Scan("Negotiate a874bg==") };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->scheme, "Negotiate");
    ASSERT_EQ(res->params.size(), 1u);
    EXPECT_EQ(res->params[0].first, "token68");
    EXPECT_EQ(res->params[0].second, "a874bg==");
}

TEST_F(ChallengeScannerTest, Scan_MultipleParams) {
    const auto res{ scanner.Scan(R"(Digest realm="testrealm@host.com", qop="auth,auth-int", nonce="dcd98b7102dd2f0e8b11d0f600bfb0c093")") };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->scheme, "Digest");
    ASSERT_EQ(res->params.size(), 3u);
    EXPECT_EQ(res->params[0].first, "realm");
    EXPECT_EQ(res->params[0].second, "testrealm@host.com");
    EXPECT_EQ(res->params[1].first, "qop");
    EXPECT_EQ(res->params[1].second, "auth,auth-int");
    EXPECT_EQ(res->params[2].first, "nonce");
    EXPECT_EQ(res->params[2].second, "dcd98b7102dd2f0e8b11d0f600bfb0c093");
}

TEST_F(ChallengeScannerTest, Formatter_FormatsCorrectly) {
    NHeaders::Challenge c1{ "Bearer", {} };
    NHeaders::Challenge c2{ "Basic", {{"realm", "Secure Area"}} };
    NHeaders::Challenge c3{ "Negotiate", {{"token68", "abc123=="}} };

    EXPECT_EQ(std::format("{}", c1), "Bearer");
    EXPECT_EQ(std::format("{}", c2), R"(Basic realm="Secure Area")");
    EXPECT_EQ(std::format("{}", c3), "Negotiate abc123==");
}

#pragma endregion

#pragma region Headers - Content-Type (ValueProxy<MimeType>)

struct ContentTypeProxyTest : testing::Test {
    Headers h{ { "content-type", R"(application/json; charset=utf-8; boundary="something")" } };
};

TEST_F(ContentTypeProxyTest, Get_ReturnsParsedMimeType) {
    const auto res{ h.ContentType().Get() };
    ASSERT_TRUE(res);
    EXPECT_EQ(res->value.type, "application");
    EXPECT_EQ(res->value.subtype, "json");
    ASSERT_EQ(res->params.size(), 2);
    EXPECT_EQ(res->params[0].first, "charset");
    EXPECT_EQ(res->params[0].second, "utf-8");
    EXPECT_EQ(res->params[1].first, "boundary");
    EXPECT_EQ(res->params[1].second, "something");
}

TEST_F(ContentTypeProxyTest, Set_FormatsMimeTypeCorrectly) {
    Headers tmp{};

    NHeaders::MimeType mime{ NHeaders::MimeTypeHeader{ "text", "html" }, {{"charset", "utf-8"}} };
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
    using enum NHeaders::AcceptEncodingEnum;

    Headers tmp{};
    std::vector<NHeaders::AcceptEncoding> encodings{ { Zstd }, { Dcb } };
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


#pragma region Headers - Connection Proxy

struct ConnectionProxyTest : testing::Test {
    Headers h{{ "connection", "keep-alive, close" }};
};

TEST_F(ConnectionProxyTest, Get_MultipleTokens_TrimsWhitespace) {
    const auto result{ h.Connection().Get() };
    ASSERT_TRUE(result);
    ASSERT_EQ(result->size(), 2u);
    EXPECT_EQ(result->at(0), "keep-alive");
    EXPECT_EQ(result->at(1), "close");
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
    const std::vector<NHeaders::MimeType> def{{ NHeaders::MimeTypeHeader{ "text", "plain" } }};

    auto res{ empty.Accept().GetWithDefault(def) };
    ASSERT_TRUE(res);
    ASSERT_EQ(res->size(), 1);
    EXPECT_EQ(res->at(0).value.type, "text");
}

TEST_F(ProxyErrorTest, ListProxy_TrySet_FailsOnInvalidInput) {
    Headers empty{};
    EXPECT_FALSE(empty.AcceptEncoding().TrySet("not_an_enum_value"));
    EXPECT_TRUE(empty.Empty());
}

#pragma endregion



#pragma region RequestHeaders - GetCookiesView

struct RequestCookiesViewTest : testing::Test {};

using StrViewPair = std::pair<std::string_view, std::string_view>;

TEST_F(RequestCookiesViewTest, GetCookiesView_ParsesStandardFormat) {
    RequestHeaders h{ Headers{ {"cookie", "session_id=12345; theme=dark; logged_in=true"} } };
    constexpr StrViewPair expected[] { { "session_id", "12345" }, { "theme", "dark" }, { "logged_in", "true" } };

    EXPECT_TRUE(std::ranges::equal(h.GetCookiesView(), expected));
}

TEST_F(RequestCookiesViewTest, GetCookiesView_HandlesMissingSpacesAndTrailingSemicolons) {
    RequestHeaders h{ Headers{ {"cookie", "a=1;b=2;; c=3;"} } };
    constexpr StrViewPair expected[] { { "a", "1" }, { "b", "2" }, { "c", "3" } };

    EXPECT_TRUE(std::ranges::equal(h.GetCookiesView(), expected));
}

TEST_F(RequestCookiesViewTest, GetCookiesView_HandlesCookiesWithoutValues) {
    RequestHeaders h{ Headers{ {"cookie", "flag_cookie; other=val"} } };
    constexpr StrViewPair expected[] { {"flag_cookie", "" }, {"other", "val" } };

    EXPECT_TRUE(std::ranges::equal(h.GetCookiesView(), expected));
}

TEST_F(RequestCookiesViewTest, GetCookiesView_EmptyOrMissingCookie_ReturnsEmptyView) {
    RequestHeaders emptyH{};
    EXPECT_EQ(std::ranges::distance(emptyH.GetCookiesView()), 0);

    RequestHeaders onlySpaces{ Headers{ {"cookie", "   ;   "} } };
    EXPECT_EQ(std::ranges::distance(onlySpaces.GetCookiesView()), 0);
}

#pragma endregion


#pragma region ResponseHeaders - GetSetCookiesView

struct ResponseSetCookiesViewTest : testing::Test {};

TEST_F(ResponseSetCookiesViewTest, GetSetCookiesView_ExtractsNameValueAndIgnoresAttributes) {
    ResponseHeaders h{{
        { "set-cookie", "session_id=12345; Path=/; Secure; HttpOnly" },
        { "set-cookie", "theme=dark; SameSite=Lax" }
    }};

    constexpr StrViewPair expected[] {
        { "session_id", "12345" },
        { "theme", "dark" }
    };

    EXPECT_TRUE(std::ranges::equal(h.GetSetCookiesView(), expected));
}

TEST_F(ResponseSetCookiesViewTest, GetSetCookiesView_HandlesMissingValuesAndUglySpacing) {
    ResponseHeaders h{ Headers{
        { "set-cookie", "  flag_cookie  ; Path=/" },
        { "set-cookie", "a=1" }
    } };

    constexpr StrViewPair expected[] {
        { "flag_cookie", "" },
        { "a", "1" }
    };

    EXPECT_TRUE(std::ranges::equal(h.GetSetCookiesView(), expected));
}

TEST_F(ResponseSetCookiesViewTest, GetSetCookiesView_EmptyReturnsEmptyView) {
    ResponseHeaders emptyH{};
    EXPECT_EQ(std::ranges::distance(emptyH.GetSetCookiesView()), 0);
}

#pragma endregion


#pragma region Headers - Link (ListProxy<Link>)

struct LinkProxyTest : testing::Test {
    Headers h{ { "link", R"(<https://example.com>; rel="preload", <https://other.com>; rel="alternate"; type="text/html")" } };
};

TEST_F(LinkProxyTest, Get_ReturnsParsedLinksAndParams) {
    const auto res{ h.Link().Get() };
    ASSERT_TRUE(res);
    ASSERT_EQ(res->size(), 2u);

    EXPECT_EQ(res->at(0).value.uri, "https://example.com");
    ASSERT_EQ(res->at(0).params.size(), 1u);
    EXPECT_EQ(res->at(0).params[0].first, "rel");
    EXPECT_EQ(res->at(0).params[0].second, "preload");

    EXPECT_EQ(res->at(1).value.uri, "https://other.com");
    ASSERT_EQ(res->at(1).params.size(), 2u);
    EXPECT_EQ(res->at(1).params[0].first, "rel");
    EXPECT_EQ(res->at(1).params[0].second, "alternate");
    EXPECT_EQ(res->at(1).params[1].first, "type");
    EXPECT_EQ(res->at(1).params[1].second, "text/html");
}

TEST_F(LinkProxyTest, Set_FormatsLinksCorrectly) {
    Headers tmp{};
    std::vector<NHeaders::Link> links{
        { NHeaders::LinkHeader{"https://a.com"}, {{"rel", "next"}} }
    };
    tmp.Link().Set(links);

    EXPECT_TRUE(tmp.Exists("link", R"(<https://a.com>;rel=next)"));
}

#pragma endregion


#pragma region RequestHeaders - Prefer & Expect (ListProxy<Parameterized<string>>)

struct FreeParameterizedProxyTest : testing::Test {
    RequestHeaders h{{
        { "prefer", "return=representation, wait=10" },
        { "expect", "100-continue; ext=val" }
    }};
};

TEST_F(FreeParameterizedProxyTest, Prefer_Get_ReturnsParsedPreferences) {
    const auto res{ h.Prefer().Get() };
    ASSERT_TRUE(res);
    ASSERT_EQ(res->size(), 2u);

    EXPECT_EQ(res->at(0).value, "return=representation");
    EXPECT_TRUE(res->at(0).params.empty());

    EXPECT_EQ(res->at(1).value, "wait=10");
    EXPECT_TRUE(res->at(1).params.empty());
}

TEST_F(FreeParameterizedProxyTest, Expect_Get_ReturnsParsedExpectations) {
    const auto res{ h.Expect().Get() };
    ASSERT_TRUE(res);
    ASSERT_EQ(res->size(), 1u);

    EXPECT_EQ(res->at(0).value, "100-continue");
    ASSERT_EQ(res->at(0).params.size(), 1u);
    EXPECT_EQ(res->at(0).params[0].first, "ext");
    EXPECT_EQ(res->at(0).params[0].second, "val");
}

#pragma endregion


#pragma region RequestHeaders - AcceptLanguage (ListProxy<Weighted<string>>)

struct AcceptLanguageProxyTest : testing::Test {
    RequestHeaders h{{ { "accept-language", "pt-BR, en-US;q=0.8, en;q=0.5" } }};
};

TEST_F(AcceptLanguageProxyTest, Get_ReturnsParsedLanguagesAndWeights) {
    const auto res{ h.AcceptLanguage().Get() };
    ASSERT_TRUE(res);
    ASSERT_EQ(res->size(), 3u);

    EXPECT_EQ(res->at(0).value, "pt-BR");
    EXPECT_DOUBLE_EQ(res->at(0).q, 1.0); // Default weight

    EXPECT_EQ(res->at(1).value, "en-US");
    EXPECT_DOUBLE_EQ(res->at(1).q, 0.8);

    EXPECT_EQ(res->at(2).value, "en");
    EXPECT_DOUBLE_EQ(res->at(2).q, 0.5);
}

TEST_F(AcceptLanguageProxyTest, Set_FormatsWeightsCorrectly) {
    RequestHeaders tmp{};
    std::vector<NHeaders::FreeWeightedHeader> langs{
        { "fr-FR", 1.0 },
        { "fr", 0.9 }
    };
    tmp.AcceptLanguage().Set(langs);

    EXPECT_TRUE(tmp.Exists("accept-language", "fr-FR,fr;q=0.900"));
}

#pragma endregion
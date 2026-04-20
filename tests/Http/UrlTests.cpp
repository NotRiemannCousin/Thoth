#include <gtest/gtest.h>
#include <Thoth/Http/Url/Url.hpp>
#include <Thoth/Http/Request/QueryParams.hpp>

using namespace Thoth::Http;


#pragma region QueryParams

struct QueryParamsTest : testing::Test {
    QueryParams params{{
        { "users",      { "Alice", "Bob", "Charlie" } },
        { "tags",       { "cpp", "test" } },
        { "page",       { "1" } }
    }};
};

TEST_F(QueryParamsTest, Exists_PresentKey_True) {
    EXPECT_TRUE(params.Exists("users"));
}

TEST_F(QueryParamsTest, Exists_MissingKey_False) {
    EXPECT_FALSE(params.Exists("missing"));
}

TEST_F(QueryParamsTest, ValExists_PresentKeyAndValue_True) {
    EXPECT_TRUE(params.ValExists("users", "Bob"));
}

TEST_F(QueryParamsTest, ValExists_PresentKeyWrongValue_False) {
    EXPECT_FALSE(params.ValExists("users", "Dave"));
}

TEST_F(QueryParamsTest, Size_CorrectCount) {
    EXPECT_EQ(params.Size(), 3u);
}

TEST_F(QueryParamsTest, Empty_False_ForNonEmpty) {
    EXPECT_FALSE(params.Empty());
}

TEST_F(QueryParamsTest, Empty_True_AfterClear) {
    QueryParams tmp{ params };
    tmp.Clear();
    EXPECT_TRUE(tmp.Empty());
    EXPECT_EQ(tmp.Size(), 0u);
}

TEST_F(QueryParamsTest, Add_NewValueToExistingKey) {
    QueryParams tmp{ params };
    tmp.Add("users", "Dave");
    EXPECT_TRUE(tmp.ValExists("users", "Dave"));
    EXPECT_EQ((*tmp.Get("users"))->size(), 4u);
}

TEST_F(QueryParamsTest, Add_NewKey_CreatesIt) {
    QueryParams tmp{ params };
    tmp.Add("lang", "en");
    EXPECT_TRUE(tmp.Exists("lang"));
    EXPECT_EQ(tmp.Size(), 4u);
}

TEST_F(QueryParamsTest, Remove_ExistingValue_ReturnsTrue) {
    QueryParams tmp{ params };
    EXPECT_TRUE(tmp.Remove("tags", "cpp"));
    EXPECT_FALSE(tmp.ValExists("tags", "cpp"));
    EXPECT_TRUE(tmp.ValExists("tags", "test"));
}

TEST_F(QueryParamsTest, Remove_MissingValue_ReturnsFalse) {
    QueryParams tmp{ params };
    EXPECT_FALSE(tmp.Remove("tags", "rust"));
}

TEST_F(QueryParamsTest, RemoveKey_ExistingKey_ReturnsTrue) {
    QueryParams tmp{ params };
    EXPECT_TRUE(tmp.RemoveKey("page"));
    EXPECT_FALSE(tmp.Exists("page"));
    EXPECT_EQ(tmp.Size(), 2u);
}

TEST_F(QueryParamsTest, RemoveKey_MissingKey_ReturnsFalse) {
    QueryParams tmp{ params };
    EXPECT_FALSE(tmp.RemoveKey("ghost"));
}

TEST_F(QueryParamsTest, SetIfNull_ExistingKey_ReturnsFalseNoChange) {
    QueryParams tmp{ params };
    EXPECT_FALSE(tmp.SetIfNull("page", "99"));
    EXPECT_TRUE(tmp.ValExists("page", "1"));
}

TEST_F(QueryParamsTest, SetIfNull_NewKey_ReturnsTrueAndSets) {
    QueryParams tmp{ params };
    EXPECT_TRUE(tmp.SetIfNull("newkey", "newval"));
    EXPECT_TRUE(tmp.ValExists("newkey", "newval"));
}

TEST_F(QueryParamsTest, Get_ExistingKey_HasValue) {
    const auto result{ params.Get("tags") };
    ASSERT_TRUE(result);
    EXPECT_EQ((*result)->size(), 2u);
}

TEST_F(QueryParamsTest, Get_MissingKey_ReturnsNullopt) {
    EXPECT_FALSE(params.Get("nope"));
}

TEST_F(QueryParamsTest, OperatorBracket_CreatesKey) {
    QueryParams tmp{ params };
    tmp["fresh"].push_back("value");
    EXPECT_TRUE(tmp.Exists("fresh"));
}

TEST_F(QueryParamsTest, Equality_SameParams_Equal) {
    QueryParams a{{ { "x", { "1" } } }};
    QueryParams b{{ { "x", { "1" } } }};
    EXPECT_EQ(a, b);
}

TEST_F(QueryParamsTest, Equality_DifferentParams_NotEqual) {
    QueryParams a{{ { "x", { "1" } } }};
    QueryParams b{{ { "x", { "2" } } }};
    EXPECT_NE(a, b);
}

TEST_F(QueryParamsTest, Iterator_CountMatchesSize) {
    std::size_t count{};
    for (const auto& pair : params) { (void)pair; count++; }
    EXPECT_EQ(count, params.Size());
}

TEST_F(QueryParamsTest, Parse_SimpleQueryString) {
    const auto p{ QueryParams::Parse("a=1&b=2") };
    EXPECT_EQ(p.Size(), 2u);
    EXPECT_TRUE(p.ValExists("a", "1"));
    EXPECT_TRUE(p.ValExists("b", "2"));
}

TEST_F(QueryParamsTest, Parse_MultipleValuesPerKey) {
    const auto p{ QueryParams::Parse("tag=cpp,test,gtest") };
    // single value "cpp,test,gtest"
    EXPECT_TRUE(p.Exists("tag"));
}

TEST_F(QueryParamsTest, Parse_EmptyString_EmptyParams) {
    const auto p{ QueryParams::Parse("") };
    EXPECT_TRUE(p.Empty());
}

TEST_F(QueryParamsTest, ParseDecodified_URLEncodedSpaces) {
    const auto result{ QueryParams::ParseDecodified("name=John%20Doe") };
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->ValExists("name", "John Doe"));
}

TEST_F(QueryParamsTest, ParseDecodified_EncodedPlus) {
    const auto result{ QueryParams::ParseDecodified("lang=C%2B%2B") };
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->ValExists("lang", "C++"));
}

TEST_F(QueryParamsTest, Format_RoundTrip) {
    QueryParams tmp{{ { "x", { "1", "2" } }, { "y", { "a" } } }};
    const auto formatted{ std::format("{}", tmp) };
    const auto reparsed{ QueryParams::Parse(formatted) };
    EXPECT_EQ(std::format("{}", reparsed), formatted);
}

#pragma endregion


#pragma region Url

struct UrlTest : testing::Test {};

TEST_F(UrlTest, Parse_SimpleHttps_Succeeds) {
    const auto r{ Url::FromUrl("https://example.com/path") };
    ASSERT_TRUE(r);
}

TEST_F(UrlTest, Parse_Scheme_IsHttps) {
    const auto r{ Url::FromUrl("https://example.com/path") };
    ASSERT_TRUE(r);
    EXPECT_EQ(r->GetScheme(), "https");
}

TEST_F(UrlTest, Parse_Host_IsExtracted) {
    const auto r{ Url::FromUrl("https://example.com/path") };
    ASSERT_TRUE(r);
    const auto auth{ r->GetAuthority() };
    ASSERT_TRUE(auth);
    EXPECT_EQ(auth->GetHostString(), "example.com");
}

TEST_F(UrlTest, Parse_Path_IsExtracted) {
    const auto r{ Url::FromUrl("https://example.com/some/path") };
    ASSERT_TRUE(r);
    EXPECT_EQ(r->GetPath(), "/some/path");
}

TEST_F(UrlTest, Parse_NoPath_GetPathOrSep_ReturnsSep) {
    const auto r{ Url::FromUrl("https://example.com") };
    ASSERT_TRUE(r);
    EXPECT_EQ(r->GetPathOrSep(), "/");
}

TEST_F(UrlTest, Parse_Port_IsExtracted) {
    const auto r{ Url::FromUrl("http://localhost:8080/") };
    ASSERT_TRUE(r);
    const auto auth{ r->GetAuthority() };
    ASSERT_TRUE(auth);
    ASSERT_TRUE(auth->port);
    EXPECT_EQ(*auth->port, 8080u);
}

TEST_F(UrlTest, Parse_Fragment_IsExtracted) {
    const auto r{ Url::FromUrl("https://example.com/page#section") };
    ASSERT_TRUE(r);
    EXPECT_EQ(r->GetFragment(), "section");
}

TEST_F(UrlTest, Parse_Query_IsExtracted) {
    const auto r{ Url::FromUrl("https://example.com/search?q=hello") };
    ASSERT_TRUE(r);
    EXPECT_EQ(r->GetQuery(), "q=hello");
}

TEST_F(UrlTest, Parse_QueryParams_HaveCorrectValues) {
    const auto r{ Url::FromUrl("https://example.com/search?id=1&type=json") };
    ASSERT_TRUE(r);
    const auto qp{ r->GetQueryParams() };
    EXPECT_TRUE(qp.ValExists("id", "1"));
    EXPECT_TRUE(qp.ValExists("type", "json"));
}

TEST_F(UrlTest, Parse_Userinfo_IsExtracted) {
    const auto r{ Url::FromUrl("http://user:pass@host.com/") };
    ASSERT_TRUE(r);
    const auto auth{ r->GetAuthority() };
    ASSERT_TRUE(auth);
    EXPECT_EQ(auth->userinfo, "user:pass");
}

TEST_F(UrlTest, Parse_FullComplex_AllFieldsPresent) {
    const auto r{ Url::FromUrl("http://admin@localhost:9000/api/v1?key=val#frag") };
    ASSERT_TRUE(r);
    EXPECT_EQ(r->GetScheme(),    "http");
    EXPECT_EQ(r->GetPath(),      "/api/v1");
    EXPECT_EQ(r->GetQuery(),     "key=val");
    EXPECT_EQ(r->GetFragment(),  "frag");
    const auto auth{ r->GetAuthority() };
    ASSERT_TRUE(auth);
    EXPECT_EQ(*auth->port, 9000u);
}

TEST_F(UrlTest, Parse_GetUrlWithoutFragment) {
    const auto r{ Url::FromUrl("https://example.com/path?q=1#frag") };
    ASSERT_TRUE(r);
    const auto noFrag{ r->GetUrlWithoutFragment() };
    EXPECT_EQ(noFrag.find('#'), std::string_view::npos);
}

TEST_F(UrlTest, Parse_Equality_SameUrl) {
    const auto a{ Url::FromUrl("https://example.com/") };
    const auto b{ Url::FromUrl("https://example.com/") };
    ASSERT_TRUE(a && b);
    EXPECT_EQ(*a, *b);
}

TEST_F(UrlTest, Parse_Inequality_DifferentUrls) {
    const auto a{ Url::FromUrl("https://example.com/") };
    const auto b{ Url::FromUrl("https://other.com/") };
    ASSERT_TRUE(a && b);
    EXPECT_NE(*a, *b);
}

TEST_F(UrlTest, Parse_RoundTrip_FormattedBackToParseable) {
    const std::string original{ "https://user@example.com:8443/a/b?c=d#e" };
    const auto r{ Url::FromUrl(original) };
    ASSERT_TRUE(r);
    const auto formatted{ std::format("{}", *r) };
    const auto reparsed{ Url::FromUrl(formatted) };
    ASSERT_TRUE(reparsed);
    EXPECT_EQ(*r, *reparsed);
}

// — Invalid URLs —

TEST_F(UrlTest, Parse_NoScheme_ReturnsError) {
    EXPECT_FALSE(Url::FromUrl("www.google.com/path"));
}

TEST_F(UrlTest, Parse_InvalidScheme_ReturnsError) {
    EXPECT_FALSE(Url::FromUrl("invalid scheme://host.com"));
}

TEST_F(UrlTest, Parse_NoAuthority_ReturnsError) {
    EXPECT_FALSE(Url::FromUrl("http:/path/only"));
}

TEST_F(UrlTest, Parse_InvalidPort_Letters_ReturnsError) {
    EXPECT_FALSE(Url::FromUrl("http://localhost:abc/"));
}

TEST_F(UrlTest, Parse_PortOverflow_ReturnsError) {
    EXPECT_FALSE(Url::FromUrl("https://localhost:65536/"));
}

TEST_F(UrlTest, Parse_EmptyHost_ReturnsError) {
    EXPECT_FALSE(Url::FromUrl("https://user@/path"));
}

// — Encode / Decode —

struct UrlEncodeTest : testing::Test {};

TEST_F(UrlEncodeTest, Encode_SpaceBecomesPct20) {
    const auto encoded{ Url::Encode("hello world") };
    EXPECT_EQ(encoded, "hello%20world");
}

TEST_F(UrlEncodeTest, Encode_PlusSign) {
    const auto encoded{ Url::Encode("C++") };
    EXPECT_EQ(encoded, "C%2B%2B");
}

TEST_F(UrlEncodeTest, Encode_PlainAlphanumeric_Unchanged) {
    const auto encoded{ Url::Encode("abc123") };
    EXPECT_EQ(encoded, "abc123");
}

TEST_F(UrlEncodeTest, Encode_Empty_ReturnsEmpty) {
    EXPECT_EQ(Url::Encode(""), "");
}

TEST_F(UrlEncodeTest, TryDecode_ValidPct20_DecodesSpace) {
    const auto result{ Url::TryDecode("hello%20world") };
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "hello world");
}

TEST_F(UrlEncodeTest, TryDecode_ValidPctPlus_DecodesPlus) {
    const auto result{ Url::TryDecode("C%2B%2B") };
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "C++");
}

TEST_F(UrlEncodeTest, TryDecode_RoundTrip_MatchesOriginal) {
    const std::string original{ "hello world! C++ <rocks>" };
    const auto encoded{ Url::Encode(original) };
    const auto decoded{ Url::TryDecode(encoded) };
    ASSERT_TRUE(decoded);
    EXPECT_EQ(*decoded, original);
}

TEST_F(UrlEncodeTest, TryDecode_InvalidSequence_ReturnsError) {
    EXPECT_FALSE(Url::TryDecode("bad%ZZsequence"));
}

TEST_F(UrlEncodeTest, TryDecode_IncompleteSequence_ReturnsError) {
    EXPECT_FALSE(Url::TryDecode("incomplete%2"));
}

// — GetDefaultPort —

struct UrlDefaultPortTest : testing::Test {};

TEST_F(UrlDefaultPortTest, DefaultPort_Http_Is80) {
    const auto port{ GetDefaultPort("http") };
    ASSERT_TRUE(port);
    EXPECT_EQ(*port, 80u);
}

TEST_F(UrlDefaultPortTest, DefaultPort_Https_Is443) {
    const auto port{ GetDefaultPort("https") };
    ASSERT_TRUE(port);
    EXPECT_EQ(*port, 443u);
}

TEST_F(UrlDefaultPortTest, DefaultPort_Unknown_ReturnsNullopt) {
    EXPECT_FALSE(GetDefaultPort("myproto"));
}

#pragma endregion
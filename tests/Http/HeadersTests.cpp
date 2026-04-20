#include <gtest/gtest.h>
#include <Thoth/Http/NHeaders/Headers.hpp>

#include <ranges>

using namespace Thoth::Http;


struct HeadersTest : testing::Test {
    Headers h1{
        { "Content-Type",   "application/json" },
        { "Authorization",  "Bearer token123"  },
        { "Accept",         "text/html"        },
        { "X-Custom",       "custom-value"     }
    };

    Headers h2{
        { "Set-Cookie",       "session=abc; Path=/" },
        { "Set-Cookie",       "theme=dark; Secure"  },
        { "Cache-Control",    "no-cache"            },
        { "WWW-Authenticate", "Basic realm=\"API\"" }
    };
};


#pragma region Exists

TEST_F(HeadersTest, Exists_CaseSensitive_True) {
    EXPECT_TRUE(h1.Exists("Content-Type"));
}

TEST_F(HeadersTest, Exists_LowerCase_True) {
    EXPECT_TRUE(h1.Exists("content-type"));
}

TEST_F(HeadersTest, Exists_UpperCase_True) {
    EXPECT_TRUE(h1.Exists("AUTHORIZATION"));
}

TEST_F(HeadersTest, Exists_MixedCase_True) {
    EXPECT_TRUE(h1.Exists("X-Custom"));
}

TEST_F(HeadersTest, Exists_Missing_False) {
    EXPECT_FALSE(h1.Exists("Non-Existent"));
}

TEST_F(HeadersTest, Exists_KeyValue_CorrectPair_True) {
    EXPECT_TRUE(h1.Exists("Content-Type", "application/json"));
}

TEST_F(HeadersTest, Exists_KeyValue_CaseInsensitiveKey_True) {
    EXPECT_TRUE(h1.Exists("content-type", "application/json"));
}

TEST_F(HeadersTest, Exists_KeyValue_WrongValue_False) {
    EXPECT_FALSE(h1.Exists("Content-Type", "text/plain"));
}

TEST_F(HeadersTest, Exists_KeyValue_MissingKey_False) {
    EXPECT_FALSE(h1.Exists("Missing", "anything"));
}

#pragma endregion


#pragma region Get

TEST_F(HeadersTest, Get_ExistingKey_HasValue) {
    const auto r{ h1.Get("Content-Type") };
    ASSERT_TRUE(r);
    EXPECT_EQ(**r, "application/json");
}

TEST_F(HeadersTest, Get_CaseInsensitive_HasValue) {
    const auto r{ h1.Get("AUTHORIZATION") };
    ASSERT_TRUE(r);
    EXPECT_EQ(**r, "Bearer token123");
}

TEST_F(HeadersTest, Get_MissingKey_ReturnsNullopt) {
    EXPECT_FALSE(h1.Get("Missing-Header"));
}

TEST_F(HeadersTest, Get_Const_ReturnsValue) {
    const Headers& ch{ h1 };
    const auto r{ ch.Get("accept") };
    ASSERT_TRUE(r);
    EXPECT_EQ(**r, "text/html");
}

#pragma endregion


#pragma region Add

TEST_F(HeadersTest, Add_NewHeader_Exists) {
    Headers tmp{ h1 };
    tmp.Add("X-Rate-Limit", "1000");
    EXPECT_TRUE(tmp.Exists("x-rate-limit", "1000"));
}

TEST_F(HeadersTest, Add_ExistingKey_AppendsValue) {
    Headers tmp{ h1 };
    tmp.Add("Accept", "application/json");
    const auto r{ tmp.Get("accept") };
    ASSERT_TRUE(r);
    // Combined value should contain both
    EXPECT_NE((*r)->find("application/json"), std::string::npos);
    EXPECT_NE((*r)->find("text/html"), std::string::npos);
}

// TEST_F(HeadersTest, Add_SetCookie_IncreasesCount) {
//     Headers tmp{ h2 };
//     tmp.Add("Set-Cookie", "new=val; Max-Age=3600");
//     const auto cookies{ tmp.GetSetCookie() };
//     EXPECT_EQ(cookies.size(), 3u);
// }

TEST_F(HeadersTest, Add_MultipleValues_SameKey_CookieSeparator) {
    Headers tmp{};
    tmp.Add("Cookie", "a=1");
    tmp.Add("Cookie", "b=2");
    const auto r{ tmp.Get("cookie") };
    ASSERT_TRUE(r);
    EXPECT_NE((*r)->find("a=1"), std::string::npos);
    EXPECT_NE((*r)->find("b=2"), std::string::npos);
}

#pragma endregion


#pragma region Set

TEST_F(HeadersTest, Set_ExistingKey_ReplacesValue) {
    Headers tmp{ h1 };
    tmp.Set("Content-Type", "text/plain");
    EXPECT_TRUE(tmp.Exists("Content-Type", "text/plain"));
    EXPECT_FALSE(tmp.Exists("Content-Type", "application/json"));
}

TEST_F(HeadersTest, Set_NewKey_AddsIt) {
    Headers tmp{ h1 };
    tmp.Set("Brand-New", "value");
    EXPECT_TRUE(tmp.Exists("brand-new", "value"));
}

// TEST_F(HeadersTest, Set_SetCookie_ReplacesAll) {
//     Headers tmp{ h2 };
//     tmp.Set("Set-Cookie", "only=one");
//     EXPECT_EQ(tmp.GetSetCookie().size(), 1u);
// }

#pragma endregion


#pragma region Remove

TEST_F(HeadersTest, Remove_ExistingPair_ReturnsTrueAndRemoves) {
    Headers tmp{ h1 };
    EXPECT_TRUE(tmp.Remove("Content-Type", "application/json"));
    EXPECT_FALSE(tmp.Exists("Content-Type"));
}

TEST_F(HeadersTest, Remove_WrongValue_ReturnsFalse) {
    Headers tmp{ h1 };
    EXPECT_FALSE(tmp.Remove("Content-Type", "wrong-value"));
    EXPECT_TRUE(tmp.Exists("Content-Type"));
}

TEST_F(HeadersTest, Remove_MissingKey_ReturnsFalse) {
    Headers tmp{ h1 };
    EXPECT_FALSE(tmp.Remove("Ghost-Header", "anything"));
}

TEST_F(HeadersTest, Remove_CaseInsensitiveKey_RemovesIt) {
    Headers tmp{ h1 };
    EXPECT_TRUE(tmp.Remove("AUTHORIZATION", "Bearer token123"));
    EXPECT_FALSE(tmp.Exists("Authorization"));
}

#pragma endregion


#pragma region SetIfNull

TEST_F(HeadersTest, SetIfNull_ExistingKey_ReturnsFalse_Unchanged) {
    Headers tmp{ h1 };
    EXPECT_FALSE(tmp.SetIfNull("Content-Type", "new-value"));
    EXPECT_TRUE(tmp.Exists("Content-Type", "application/json"));
}

TEST_F(HeadersTest, SetIfNull_NewKey_ReturnsTrueAndSets) {
    Headers tmp{ h1 };
    EXPECT_TRUE(tmp.SetIfNull("Fresh-Header", "fresh-value"));
    EXPECT_TRUE(tmp.Exists("fresh-header", "fresh-value"));
}

TEST_F(HeadersTest, SetIfNull_CaseInsensitive_ExistingKey_ReturnsFalse) {
    Headers tmp{ h1 };
    EXPECT_FALSE(tmp.SetIfNull("CONTENT-TYPE", "anything"));
}

#pragma endregion


#pragma region Size / Empty / Clear

TEST_F(HeadersTest, Size_CorrectCount) {
    EXPECT_EQ(h1.Size(), 4u);
}

TEST_F(HeadersTest, Empty_False_ForNonEmpty) {
    EXPECT_FALSE(h1.Empty());
}

TEST_F(HeadersTest, Empty_True_ForDefaultConstructed) {
    Headers empty{};
    EXPECT_TRUE(empty.Empty());
    EXPECT_EQ(empty.Size(), 0u);
}

TEST_F(HeadersTest, Clear_SetsEmptyAndSizeZero) {
    Headers tmp{ h1 };
    tmp.Clear();
    EXPECT_TRUE(tmp.Empty());
    EXPECT_EQ(tmp.Size(), 0u);
    EXPECT_FALSE(tmp.Exists("Content-Type"));
}

#pragma endregion


#pragma region GetSetCookie / GetSetCookieView

// TEST_F(HeadersTest, GetSetCookie_ReturnsAllCookies) {
//     const auto cookies{ h2.GetSetCookie() };
//     EXPECT_EQ(cookies.size(), 2u);
//     EXPECT_NE(std::ranges::find(cookies, "session=abc; Path=/"), cookies.end());
//     EXPECT_NE(std::ranges::find(cookies, "theme=dark; Secure"),  cookies.end());
// }
//
// TEST_F(HeadersTest, GetSetCookie_NoCookies_ReturnsEmpty) {
//     const auto cookies{ h1.GetSetCookie() };
//     EXPECT_TRUE(cookies.empty());
// }
//
// TEST_F(HeadersTest, GetSetCookieView_IteratesAll) {
//     std::vector<std::string> collected;
//     for (const auto& c : h2.GetSetCookieView())
//         collected.emplace_back(c);
//     EXPECT_EQ(collected.size(), 2u);
// }

#pragma endregion


#pragma region operator[]

TEST_F(HeadersTest, OperatorBracket_ExistingKey_ReturnsValue) {
    Headers tmp{ h1 };
    EXPECT_EQ(tmp["content-type"], "application/json");
}

TEST_F(HeadersTest, OperatorBracket_CaseInsensitive_ExistingKey) {
    Headers tmp{ h1 };
    EXPECT_EQ(tmp["ACCEPT"], "text/html");
}

TEST_F(HeadersTest, OperatorBracket_NewKey_CreatesAndAssigns) {
    Headers tmp{ h1 };
    tmp["new-key"] = "new-val";
    EXPECT_TRUE(tmp.Exists("new-key", "new-val"));
}

#pragma endregion


#pragma region Equality

TEST_F(HeadersTest, Equality_SameContents_Equal) {
    const Headers copy{ h1 };
    EXPECT_EQ(h1, copy);
}

TEST_F(HeadersTest, Equality_DifferentContents_NotEqual) {
    Headers modified{ h1 };
    modified.Add("Extra", "extra-val");
    EXPECT_NE(h1, modified);
}

#pragma endregion


#pragma region Iteration - keys are lowercased

TEST_F(HeadersTest, Iterator_Count_MatchesSize) {
    std::size_t count{};
    for (const auto& pair : h1) { (void)pair; count++; }
    EXPECT_EQ(count, h1.Size());
}

TEST_F(HeadersTest, Iterator_KeysAreAllLowercase) {
    for (const auto& key : h1 | std::views::keys) {
        for (const char c : key)
            EXPECT_FALSE('A' <= c && c <= 'Z') << "Upper-case letter in key: " << key;
    }
}

#pragma endregion


#pragma region Formatting

TEST_F(HeadersTest, Format_SimpleHeaders_ExactOutput) {
    Headers simple{
        { "content-type", "text/html" },
        { "server",       "nginx/1.18" }
    };
    const auto out{ std::format("{}", simple) };
    EXPECT_EQ(out, "content-type: text/html\r\nserver: nginx/1.18\r\n");
}

TEST_F(HeadersTest, Format_DuplicateKey_EachOnOwnLine) {
    Headers rep{
        { "accept-encoding", "gzip"    },
        { "accept-encoding", "deflate" }
    };
    const auto out{ std::format("{}", rep) };
    EXPECT_EQ(out, "accept-encoding: gzip\r\naccept-encoding: deflate\r\n");
}

TEST_F(HeadersTest, Format_SetCookie_EachOnOwnLine) {
    Headers sc{
        { "set-cookie", "a=1" },
        { "set-cookie", "b=2" }
    };
    EXPECT_EQ(std::format("{}", sc), "set-cookie: a=1\r\nset-cookie: b=2\r\n");
}

#pragma endregion


#pragma region Parse (from raw string)

TEST_F(HeadersTest, Parse_ValidHeaders_Succeeds) {
    const auto result{ Headers::Parse("content-type: application/json\r\nauthorization: Bearer tok\r\n") };
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->Exists("content-type", "application/json"));
    EXPECT_TRUE(result->Exists("authorization", "Bearer tok"));
}

TEST_F(HeadersTest, Parse_WhitespaceAroundValue_IsTrimmed) {
    const auto result{ Headers::Parse("content-type:    text/html   \r\n") };
    ASSERT_TRUE(result);
    EXPECT_EQ(**result->Get("content-type"), "text/html");
}

TEST_F(HeadersTest, Parse_LeadingSpaceInKey_BadRequest) {
    const auto result{ Headers::Parse("   content-type: application/json\r\n") };
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error(), StatusCodeEnum::BadRequest);
}

TEST_F(HeadersTest, Parse_EmptyValue_Succeeds) {
    const auto result{ Headers::Parse("x-empty:    \r\n") };
    ASSERT_TRUE(result);
    EXPECT_TRUE(result->Exists("x-empty"));
}

// TEST_F(HeadersTest, Parse_MultipleCookies_AllPresent) {
//     std::string_view raw{
//         "set-cookie: a=1\r\n"
//         "set-cookie: b=2\r\n"
//     };
//     const auto result{ Headers::Parse(raw) };
//     ASSERT_TRUE(result);
//     const auto cookies{ result->GetSetCookie() };
//     EXPECT_EQ(cookies.size(), 2u);
// }

TEST_F(HeadersTest, Parse_DefaultHeaders_NotEmpty) {
    const auto dh{ Headers::DefaultHeaders() };
    EXPECT_FALSE(dh.Empty());

    const auto agent{ dh.Get("user-agent") };
    ASSERT_TRUE(agent);
    EXPECT_EQ(**agent, "Thoth/0.1");
}

TEST_F(HeadersTest, ConstructFromVector_LowercasesKeys) {
    const Headers::MapType vec{
        { "Host", "example.com" },
        { "User-Agent", "TestClient/1.0" }
    };
    const Headers fromVec{ vec };
    EXPECT_TRUE(fromVec.Exists("host"));
    EXPECT_TRUE(fromVec.Exists("user-agent"));
    EXPECT_EQ(fromVec.Size(), 2u);
}

#pragma endregion
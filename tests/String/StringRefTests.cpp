#include <gtest/gtest.h>
#include <Thoth/NJson/StringRef.hpp>

#include <algorithm>
#include <cctype>
#include <format>
#include <ranges>
#include <string>
#include <string_view>

using Thoth::NJson::StringRef;


#pragma region Construction & Conversion

struct StringRefConstructTest : testing::Test {};

TEST_F(StringRefConstructTest, DefaultConstruct_EmptyStr) {
    const StringRef ref{};
    EXPECT_EQ(ref.AsStringView(), "");
}

TEST_F(StringRefConstructTest, FromString_StrViewMatches) {
    const std::string s{ "hello" };
    const StringRef ref{ s };
    const std::string_view sv{ ref };
    EXPECT_EQ(sv, "hello");
}

TEST_F(StringRefConstructTest, ConvertToStringView_ImplicitConversion) {
    const std::string s{ "world" };
    const StringRef ref{ s };
    const std::string_view sv = ref;
    EXPECT_EQ(sv, "world");
}

TEST_F(StringRefConstructTest, ConvertToString_ImplicitConversion) {
    const std::string s{ "copy me" };
    const StringRef ref{ s };
    const std::string result = ref;
    EXPECT_EQ(result, "copy me");
}

TEST_F(StringRefConstructTest, CopyConstruct_SameStr) {
    const std::string s{ "data" };
    const StringRef a{ s };
    const StringRef b{ a };
    EXPECT_EQ(static_cast<std::string_view>(b), "data");
}

TEST_F(StringRefConstructTest, MoveConstruct_Succeeds) {
    const std::string s{ "move" };
    StringRef a{ s };
    const StringRef b{ std::move(a) };
    EXPECT_EQ(static_cast<std::string_view>(b), "move");
}

TEST_F(StringRefConstructTest, FromOwned_CreatesValidRef) {
    const StringRef ref{ StringRef::FromOwned(std::string("owned")) };
    EXPECT_EQ(ref.AsStringView(), "owned");
}

#pragma endregion


#pragma region Equality

struct StringRefEqualityTest : testing::Test {
    const std::string s1{ "same" };
    const std::string s2{ "same" };
    const std::string s3{ "diff" };
    const StringRef r1{ s1 };
    const StringRef r2{ s2 };
    const StringRef r3{ s3 };
};

TEST_F(StringRefEqualityTest, SameContent_Equal) {
    EXPECT_EQ(r1, r2);
}

TEST_F(StringRefEqualityTest, DifferentContent_NotEqual) {
    EXPECT_NE(r1, r3);
}

TEST_F(StringRefEqualityTest, SelfEquality_Equal) {
    EXPECT_EQ(r1, r1);
}

TEST_F(StringRefEqualityTest, EmptyRefs_Equal) {
    const StringRef empty1{};
    const StringRef empty2{};
    EXPECT_EQ(empty1, empty2);
}

#pragma endregion


#pragma region AsStringView

struct StringRefAsViewTest : testing::Test {};

TEST_F(StringRefAsViewTest, AsStringView_ReturnsCorrectView) {
    const std::string s{ "hello world" };
    const StringRef ref{ s };
    EXPECT_EQ(ref.AsStringView(), "hello world");
}

TEST_F(StringRefAsViewTest, AsStringView_EmptyRef_ReturnsEmpty) {
    const StringRef ref{};
    EXPECT_TRUE(ref.AsStringView().empty());
}

TEST_F(StringRefAsViewTest, AsStringView_MatchesImplicitConversion) {
    const std::string s{ "test" };
    const StringRef ref{ s };
    const std::string_view explicitView{ ref.AsStringView() };
    const std::string_view implicitView{ ref };
    EXPECT_EQ(explicitView, implicitView);
}

#pragma endregion


#pragma region Range Interface

struct StringRefRangeTest : testing::Test {
    const std::string s{ "hello" };
    const StringRef ref{ s };
};

TEST_F(StringRefRangeTest, RangeFor_IteratesCharsInOrder) {
    std::string collected;
    for (const auto c : ref)
        collected += c;
    EXPECT_EQ(collected, "hello");
}

TEST_F(StringRefRangeTest, BeginEnd_MatchStringViewIterators) {
    EXPECT_EQ(ref.begin(), ref.AsStringView().begin());
    EXPECT_EQ(ref.end(), ref.AsStringView().end());
}

TEST_F(StringRefRangeTest, CBeginCEnd_ConstIteration) {
    std::string collected;
    for (auto it{ ref.cbegin() }; it != ref.cend(); ++it)
        collected += *it;
    EXPECT_EQ(collected, "hello");
}

TEST_F(StringRefRangeTest, Size_ReturnsLength) {
    EXPECT_EQ(ref.size(), 5u);
}

TEST_F(StringRefRangeTest, Empty_FalseForNonEmpty) {
    EXPECT_FALSE(ref.empty());
}

TEST_F(StringRefRangeTest, Empty_TrueForDefaultConstructed) {
    const StringRef empty{};
    EXPECT_TRUE(empty.empty());
}

TEST_F(StringRefRangeTest, Data_ReturnsPointerToChars) {
    EXPECT_EQ((std::string_view{ ref.data(), ref.size() }), "hello");
}

TEST_F(StringRefRangeTest, Front_ReturnsFirstChar) {
    EXPECT_EQ(ref.front(), 'h');
}

TEST_F(StringRefRangeTest, Back_ReturnsLastChar) {
    EXPECT_EQ(ref.back(), 'o');
}

TEST_F(StringRefRangeTest, IndexAccess_Forward) {
    EXPECT_EQ(ref[0], 'h');
    EXPECT_EQ(ref[2], 'l');
    EXPECT_EQ(ref[4], 'o');
}

TEST_F(StringRefRangeTest, Contains_Substring_True) {
    EXPECT_TRUE(ref.contains(std::string_view{ "ell" }));
    EXPECT_TRUE(ref.contains("ell"));
}

TEST_F(StringRefRangeTest, Contains_Substring_False) {
    EXPECT_FALSE(ref.contains(std::string_view{ "xyz" }));
    EXPECT_FALSE(ref.contains("xyz"));
}

TEST_F(StringRefRangeTest, Contains_Char_True) {
    EXPECT_TRUE(ref.contains('h'));
    EXPECT_TRUE(ref.contains('o'));
}

TEST_F(StringRefRangeTest, Contains_Char_False) {
    EXPECT_FALSE(ref.contains('z'));
}

TEST_F(StringRefRangeTest, ReverseIterators_ReversedOrder) {
    std::string collected;
    for (auto it{ ref.rbegin() }; it != ref.rend(); ++it)
        collected += *it;
    EXPECT_EQ(collected, "olleh");
}

TEST_F(StringRefRangeTest, RangesDistance_MatchesSize) {
    EXPECT_EQ(std::ranges::distance(ref), 5);
}

TEST_F(StringRefRangeTest, RangesFilterView_YieldsVowels) {
    auto vowels{ ref | std::views::filter([](char c) {
        return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u';
    }) };
    std::string collected;
    for (const auto c : vowels)
        collected += c;
    EXPECT_EQ(collected, "eo");
}

TEST_F(StringRefRangeTest, RangesTransformView_UpperCase) {
    auto upper{ ref | std::views::transform([](char c) {
        return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }) };
    std::string collected;
    for (const auto c : upper)
        collected += c;
    EXPECT_EQ(collected, "HELLO");
}

#pragma endregion


#pragma region Formatting

struct StringRefFormatTest : testing::Test {};

TEST_F(StringRefFormatTest, Format_PlainString) {
    const std::string s{ "hello" };
    const StringRef ref{ s };
    EXPECT_EQ(std::format("{}", ref), "hello");
}

TEST_F(StringRefFormatTest, Format_EmptyString) {
    const StringRef ref{};
    EXPECT_EQ(std::format("{}", ref), "");
}

TEST_F(StringRefFormatTest, Format_StringWithSpaces) {
    const std::string s{ "hello world" };
    const StringRef ref{ s };
    EXPECT_EQ(std::format("{}", ref), "hello world");
}

TEST_F(StringRefFormatTest, Format_WithFormatSpec_IgnoresSpec) {
    const std::string s{ "abc" };
    const StringRef ref{ s };
    EXPECT_EQ(std::format("{:s}", ref), "abc");
}

#pragma endregion

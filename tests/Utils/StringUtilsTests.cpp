#include <gtest/gtest.h>
#include <Thoth/String/Utils.hpp>

using namespace Thoth::String;

#pragma region CaseInsensitiveCompare

struct CaseInsensitiveCompareTest : testing::Test {};

TEST_F(CaseInsensitiveCompareTest, SameLower_ReturnsTrue) {
    EXPECT_TRUE(CaseInsensitiveCompare('a', 'a'));
}

TEST_F(CaseInsensitiveCompareTest, SameUpper_ReturnsTrue) {
    EXPECT_TRUE(CaseInsensitiveCompare('Z', 'Z'));
}

TEST_F(CaseInsensitiveCompareTest, LowerVsUpper_SameLetter_ReturnsTrue) {
    EXPECT_TRUE(CaseInsensitiveCompare('a', 'A'));
    EXPECT_TRUE(CaseInsensitiveCompare('A', 'a'));
}

TEST_F(CaseInsensitiveCompareTest, DifferentLetters_ReturnsFalse) {
    EXPECT_FALSE(CaseInsensitiveCompare('a', 'b'));
    EXPECT_FALSE(CaseInsensitiveCompare('A', 'B'));
}

TEST_F(CaseInsensitiveCompareTest, DigitVsLetter_ReturnsFalse) {
    EXPECT_FALSE(CaseInsensitiveCompare('1', 'a'));
}

TEST_F(CaseInsensitiveCompareTest, SameDigit_ReturnsTrue) {
    EXPECT_TRUE(CaseInsensitiveCompare('5', '5'));
}

#pragma endregion


#pragma region Trim (in-place, modifies string_view)

struct TrimTest : testing::Test {};

TEST_F(TrimTest, Trim_BothSides_RemovesSpaces) {
    std::string_view sv{ "  hello  " };
    Trim(sv);
    EXPECT_EQ(sv, "hello");
}

TEST_F(TrimTest, Trim_LeadingOnly) {
    std::string_view sv{ "  hello" };
    Trim(sv);
    EXPECT_EQ(sv, "hello");
}

TEST_F(TrimTest, Trim_TrailingOnly) {
    std::string_view sv{ "hello  " };
    Trim(sv);
    EXPECT_EQ(sv, "hello");
}

TEST_F(TrimTest, Trim_NoWhitespace_Unchanged) {
    std::string_view sv{ "hello" };
    Trim(sv);
    EXPECT_EQ(sv, "hello");
}

TEST_F(TrimTest, Trim_AllWhitespace_EmptyResult) {
    std::string_view sv{ "   \t  " };
    Trim(sv);
    EXPECT_EQ(sv, "");
}

TEST_F(TrimTest, Trim_Empty_StaysEmpty) {
    std::string_view sv{ "" };
    Trim(sv);
    EXPECT_EQ(sv, "");
}

TEST_F(TrimTest, Trim_TabsAndNewlines) {
    std::string_view sv{ "\t\n hello \r\n" };
    Trim(sv);
    EXPECT_EQ(sv, "hello");
}

TEST_F(TrimTest, Trim_CustomChars) {
    std::string_view sv{ "***hello***" };
    Trim(sv, "*");
    EXPECT_EQ(sv, "hello");
}

TEST_F(TrimTest, LeftTrim_OnlyLeading) {
    std::string_view sv{ "  hello  " };
    LeftTrim(sv);
    EXPECT_EQ(sv, "hello  ");
}

TEST_F(TrimTest, RightTrim_OnlyTrailing) {
    std::string_view sv{ "  hello  " };
    RightTrim(sv);
    EXPECT_EQ(sv, "  hello");
}

#pragma endregion


#pragma region Trimmed (non-mutating, returns new string_view)

struct TrimmedTest : testing::Test {};

TEST_F(TrimmedTest, Trimmed_DoesNotModifyOriginal) {
    const std::string_view original{ "  hello  " };
    const auto trimmed{ Trimmed(original) };
    EXPECT_EQ(trimmed, "hello");
    EXPECT_EQ(original, "  hello  ");
}

TEST_F(TrimmedTest, LeftTrimmed_DoesNotModifyOriginal) {
    const std::string_view original{ "  hello  " };
    const auto result{ LeftTrimmed(original) };
    EXPECT_EQ(result, "hello  ");
    EXPECT_EQ(original, "  hello  ");
}

TEST_F(TrimmedTest, RightTrimmed_DoesNotModifyOriginal) {
    const std::string_view original{ "  hello  " };
    const auto result{ RightTrimmed(original) };
    EXPECT_EQ(result, "  hello");
    EXPECT_EQ(original, "  hello  ");
}

TEST_F(TrimmedTest, Trimmed_InternalSpaces_Preserved) {
    const auto r{ Trimmed("  a b c  ") };
    EXPECT_EQ(r, "a b c");
}

#pragma endregion


#pragma region TrimmedStr (returns std::string)

struct TrimmedStrTest : testing::Test {};

TEST_F(TrimmedStrTest, TrimmedStr_ReturnsStdString) {
    const std::string r{ TrimmedStr("  hello  ") };
    EXPECT_EQ(r, "hello");
}

TEST_F(TrimmedStrTest, LeftTrimmedStr_ReturnsStdString) {
    EXPECT_EQ(LeftTrimmedStr("  hi  "), "hi  ");
}

TEST_F(TrimmedStrTest, RightTrimmedStr_ReturnsStdString) {
    EXPECT_EQ(RightTrimmedStr("  hi  "), "  hi");
}

#pragma endregion


#pragma region IsVisible

struct IsVisibleTest : testing::Test {};

TEST_F(IsVisibleTest, PrintableAscii_IsVisible) {
    EXPECT_TRUE(IsVisible('a'));
    EXPECT_TRUE(IsVisible('Z'));
    EXPECT_TRUE(IsVisible('5'));
    EXPECT_TRUE(IsVisible('!'));
    EXPECT_TRUE(IsVisible('~'));
}

TEST_F(IsVisibleTest, Space_NotVisible) {
    EXPECT_FALSE(IsVisible(' '));
}

TEST_F(IsVisibleTest, ControlChars_NotVisible) {
    EXPECT_FALSE(IsVisible('\n'));
    EXPECT_FALSE(IsVisible('\t'));
    EXPECT_FALSE(IsVisible('\r'));
    EXPECT_FALSE(IsVisible('\0'));
}

TEST_F(IsVisibleTest, Delete_NotVisible) {
    EXPECT_FALSE(IsVisible('\x7f'));
}

#pragma endregion


#pragma region CharSequences - spot checks

struct CharSequencesTest : testing::Test {};

TEST_F(CharSequencesTest, Digits_ContainsAllDigits) {
    for (char c{ '0' }; c <= '9'; ++c)
        EXPECT_NE(CharSequences::digits.find(c), std::string_view::npos) << c;
}

TEST_F(CharSequencesTest, Alpha_ContainsAllLetters) {
    for (char c{ 'a' }; c <= 'z'; ++c)
        EXPECT_NE(CharSequences::alpha.find(c), std::string_view::npos) << c;
    for (char c{ 'A' }; c <= 'Z'; ++c)
        EXPECT_NE(CharSequences::alpha.find(c), std::string_view::npos) << c;
}

TEST_F(CharSequencesTest, AlphaLower_NoUpperCase) {
    for (char c : CharSequences::alphaLower)
        EXPECT_FALSE('A' <= c && c <= 'Z');
}

TEST_F(CharSequencesTest, AlphaUpper_NoLowerCase) {
    for (char c : CharSequences::alphaUpper)
        EXPECT_FALSE('a' <= c && c <= 'z');
}

TEST_F(CharSequencesTest, Alphanumeric_ContainsDigitsAndLetters) {
    EXPECT_NE(CharSequences::alphanumeric.find('a'), std::string_view::npos);
    EXPECT_NE(CharSequences::alphanumeric.find('Z'), std::string_view::npos);
    EXPECT_NE(CharSequences::alphanumeric.find('5'), std::string_view::npos);
}

TEST_F(CharSequencesTest, Base64_HasExactly64Chars) {
    EXPECT_EQ(CharSequences::base64.size(), 64u);
}

TEST_F(CharSequencesTest, Base64Url_HasExactly64Chars) {
    EXPECT_EQ(CharSequences::base64Url.size(), 64u);
}

TEST_F(CharSequencesTest, Base64Url_HasDashAndUnderscore) {
    EXPECT_NE(CharSequences::base64Url.find('-'), std::string_view::npos);
    EXPECT_NE(CharSequences::base64Url.find('_'), std::string_view::npos);
}

TEST_F(CharSequencesTest, Hex_ContainsDigitsAndAF) {
    EXPECT_NE(CharSequences::hex.find('a'), std::string_view::npos);
    EXPECT_NE(CharSequences::hex.find('F'), std::string_view::npos);
}

TEST_F(CharSequencesTest, Whitespace_ContainsCommonChars) {
    EXPECT_NE(CharSequences::whitespace.find(' '),  std::string_view::npos);
    EXPECT_NE(CharSequences::whitespace.find('\t'), std::string_view::npos);
    EXPECT_NE(CharSequences::whitespace.find('\n'), std::string_view::npos);
}

TEST_F(CharSequencesTest, Newlines_ContainsCRLF) {
    EXPECT_NE(CharSequences::newlines.find('\r'), std::string_view::npos);
    EXPECT_NE(CharSequences::newlines.find('\n'), std::string_view::npos);
}

TEST_F(CharSequencesTest, Http_Tchar_ContainsAlphanumeric) {
    EXPECT_NE(CharSequences::Http::tchar.find('a'), std::string_view::npos);
    EXPECT_NE(CharSequences::Http::tchar.find('0'), std::string_view::npos);
    EXPECT_NE(CharSequences::Http::tchar.find('!'), std::string_view::npos);
}

#pragma endregion


#pragma region MakeBitset

struct MakeBitsetTest : testing::Test {};

TEST_F(MakeBitsetTest, MakeBitset_SingleString_SetsBitsForEachChar) {
    constexpr auto bs{ MakeBitset({ "abc" }) };
    EXPECT_TRUE(bs.test('a'));
    EXPECT_TRUE(bs.test('b'));
    EXPECT_TRUE(bs.test('c'));
}

TEST_F(MakeBitsetTest, MakeBitset_CharNotInSet_NotSet) {
    constexpr auto bs{ MakeBitset({ "abc" }) };
    EXPECT_FALSE(bs.test('d'));
    EXPECT_FALSE(bs.test('z'));
}

TEST_F(MakeBitsetTest, MakeBitset_MultipleStrings_UnionOfChars) {
    constexpr auto bs{ MakeBitset({ "ab", "cd" }) };
    EXPECT_TRUE(bs.test('a'));
    EXPECT_TRUE(bs.test('d'));
    EXPECT_FALSE(bs.test('e'));
}

TEST_F(MakeBitsetTest, MakeBitset_Digits_AllDigitsSet) {
    constexpr auto bs{ MakeBitset({ CharSequences::digits }) };
    for (char c{ '0' }; c <= '9'; ++c)
        EXPECT_TRUE(bs.test(static_cast<unsigned char>(c)));
}

#pragma endregion
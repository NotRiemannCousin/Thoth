#include <gtest/gtest.h>
#include <Thoth/String/UnicodeViewer.hpp>

using namespace Thoth::String;


#pragma region Utf8View - ASCII

struct Utf8ViewAsciiTest : testing::Test {};

TEST_F(Utf8ViewAsciiTest, SingleAsciiChar_DecodesCorrectly) {
    const std::u8string s{ u8"A" };
    Utf8View view{ s };
    auto it{ view.begin() };
    EXPECT_EQ(*it, U'A');
}

TEST_F(Utf8ViewAsciiTest, AsciiString_AllCharsDecoded) {
    const std::u8string s{ u8"abc" };
    Utf8View view{ s };
    std::u32string result;
    for (auto it{ view.begin() }; it != view.end(); ++it)
        result += *it;
    EXPECT_EQ(result, U"abc");
}

TEST_F(Utf8ViewAsciiTest, EmptyString_BeginEqualsEnd) {
    const std::u8string s{};
    Utf8View view{ s };
    EXPECT_EQ(view.begin(), view.end());
}

TEST_F(Utf8ViewAsciiTest, IsValid_AsciiString_True) {
    EXPECT_TRUE(Utf8View::IsValid(u8"hello"));
}

#pragma endregion


#pragma region Utf8View - Multibyte

struct Utf8ViewMultibyteTest : testing::Test {};

TEST_F(Utf8ViewMultibyteTest, TwoByteChar_DecodesCorrectly) {
    // U+00E9 = é = 0xC3 0xA9 in UTF-8
    const std::u8string s{ u8"\u00E9" };
    Utf8View view{ s };
    auto it{ view.begin() };
    EXPECT_EQ(*it, U'\u00E9');
}

TEST_F(Utf8ViewMultibyteTest, ThreeByteChar_DecodesCorrectly) {
    // U+4E2D = 中 = 0xE4 0xB8 0xAD in UTF-8
    const std::u8string s{ u8"\u4E2D" };
    Utf8View view{ s };
    auto it{ view.begin() };
    EXPECT_EQ(*it, U'\u4E2D');
}

TEST_F(Utf8ViewMultibyteTest, MixedAsciiAndMultibyte_CountChars) {
    // "aé" = 'a' + U+00E9
    EXPECT_EQ(std::ranges::distance(Utf8View{ u8"a\u00E9" }), 2u);
}

TEST_F(Utf8ViewMultibyteTest, IsValid_MultibyteString_True) {
    EXPECT_TRUE(Utf8View::IsValid(u8"\u00E9\u4E2D"));
}

TEST_F(Utf8ViewMultibyteTest, InvalidByte_YieldsUnknownChar) {
    // 0xFF is always invalid in UTF-8
    std::u8string s{};
    s.push_back('\xFF');

    Utf8View view{ s };
    auto it{ view.begin() };
    EXPECT_EQ(*it, UnknownChar);
}

TEST_F(Utf8ViewMultibyteTest, IsValid_InvalidByte_False) {
    std::u8string s{};
    s.push_back('\xFF');

    EXPECT_FALSE(Utf8View::IsValid(s));
}

#pragma endregion


#pragma region Utf8View - ConvertTo

struct Utf8ViewConvertTest : testing::Test {};

TEST_F(Utf8ViewConvertTest, ConvertTo_Utf32_MatchesCodepoints) {
    const std::u8string src{ u8"hello" };
    const auto result{ Utf8View::ConvertTo<char32_t>(src) };
    EXPECT_EQ(result, U"hello");
}

TEST_F(Utf8ViewConvertTest, ConvertTo_Utf16_AsciiRoundtrip) {
    const std::u8string src{ u8"abc" };
    const auto utf16{ Utf8View::ConvertTo<char16_t>(src) };
    const auto backToUtf32{ Utf8View::ConvertTo<char32_t>(src) };
    EXPECT_EQ(backToUtf32.size(), 3u);
}

TEST_F(Utf8ViewConvertTest, ConvertTo_Empty_ReturnsEmpty) {
    const std::u8string src{};
    const auto result{ Utf8View::ConvertTo<char32_t>(src) };
    EXPECT_TRUE(result.empty());
}

#pragma endregion


#pragma region Utf16View

struct Utf16ViewTest : testing::Test {};

TEST_F(Utf16ViewTest, AsciiString_AllCharsDecoded) {
    const std::u16string s{ u"ABC" };
    Utf16View view{ s };
    std::u32string result;
    for (auto it{ view.begin() }; it != view.end(); ++it)
        result += *it;
    EXPECT_EQ(result, U"ABC");
}

TEST_F(Utf16ViewTest, BmpChar_DecodesCorrectly) {
    // U+4E2D = 中, BMP range, single UTF-16 unit
    const std::u16string s{ u"\u4E2D" };
    Utf16View view{ s };
    auto it{ view.begin() };
    EXPECT_EQ(*it, U'\u4E2D');
}

TEST_F(Utf16ViewTest, IsValid_AsciiU16_True) {
    EXPECT_TRUE(Utf16View::IsValid(u"hello"));
}

TEST_F(Utf16ViewTest, EmptyString_BeginEqualsEnd) {
    const std::u16string s{};
    Utf16View view{ s };
    EXPECT_EQ(view.begin(), view.end());
}

#pragma endregion


#pragma region Utf32View

struct Utf32ViewTest : testing::Test {};

TEST_F(Utf32ViewTest, AsciiString_AllCharsDecoded) {
    const std::u32string s{ U"hello" };
    Utf32View view{ s };
    std::u32string result;
    for (auto it{ view.begin() }; it != view.end(); ++it)
        result += *it;
    EXPECT_EQ(result, U"hello");
}

TEST_F(Utf32ViewTest, SingleCodepoint_DecodesCorrectly) {
    const std::u32string s{ U"\U0001F600" };
    Utf32View view{ s };
    auto it{ view.begin() };
    EXPECT_EQ(*it, U'\U0001F600');
}

TEST_F(Utf32ViewTest, IsValid_ValidCodepoints_True) {
    EXPECT_TRUE(Utf32View::IsValid(U"hello"));
}

TEST_F(Utf32ViewTest, IsValid_AboveMaxCodepoint_False) {
    // 0x110000 is above U+10FFFF
    const std::u32string s{ char32_t(0x110000), 1 };
    EXPECT_FALSE(Utf32View::IsValid(s));
}

TEST_F(Utf32ViewTest, EmptyString_BeginEqualsEnd) {
    const std::u32string s{};
    Utf32View view{ s };
    EXPECT_EQ(view.begin(), view.end());
}

#pragma endregion
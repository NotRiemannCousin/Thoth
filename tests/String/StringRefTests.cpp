#include <gtest/gtest.h>
#include <Thoth/NJson/StringRef.hpp>

using Thoth::NJson::StringRef;


#pragma region Construction & Conversion

struct StringRefConstructTest : testing::Test {};

TEST_F(StringRefConstructTest, DefaultConstruct_EmptyStr) {
    const StringRef ref{};
    EXPECT_EQ(ref.str, "");
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

#pragma endregion
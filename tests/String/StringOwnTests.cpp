#include <gtest/gtest.h>
#include <Thoth/NJson/StringOwn.hpp>
#include <Thoth/NJson/StringRef.hpp>

#include <algorithm>
#include <cctype>
#include <format>
#include <ranges>
#include <string>
#include <string_view>

using Thoth::NJson::StringOwn;
using Thoth::NJson::StringRef;


#pragma region Construction

struct StringOwnConstructTest : testing::Test {};

TEST_F(StringOwnConstructTest, DefaultConstruct_Empty) {
    const StringOwn own{};
    EXPECT_TRUE(own.view().empty());
}

TEST_F(StringOwnConstructTest, FromStdString_OwnsContent) {
    const std::string s{ "hello" };
    const StringOwn own{ s };
    EXPECT_EQ(own.view(), "hello");
}

TEST_F(StringOwnConstructTest, FromStringView_OwnsContent) {
    const StringOwn own{ std::string_view{ "world" } };
    EXPECT_EQ(own.view(), "world");
}

TEST_F(StringOwnConstructTest, FromCString_OwnsContent) {
    const StringOwn own{ "hello" };
    EXPECT_EQ(own.view(), "hello");
}

TEST_F(StringOwnConstructTest, FromNullCString_Empty) {
    const StringOwn own{ nullptr };
    EXPECT_TRUE(own.view().empty());
}

TEST_F(StringOwnConstructTest, FromStringRef_OwnsCopy) {
    const std::string s{ "refdata" };
    const StringRef ref{ s };
    const StringOwn own{ ref };
    EXPECT_EQ(own.view(), "refdata");
}

TEST_F(StringOwnConstructTest, FromString_Rvalue_IsMoved) {
    std::string s{ "rvalue" };
    const StringOwn own{ std::move(s) };
    EXPECT_EQ(own.view(), "rvalue");
    // NOLINTNEXTLINE(bugprone-use-after-move, clang-analyzer-cplusplus.Move)
    EXPECT_TRUE(s.empty());
}

TEST_F(StringOwnConstructTest, CopyConstruct_IndependentCopy) {
    const StringOwn original{ std::string("data") };
    const StringOwn copy{ original };
    EXPECT_EQ(copy.view(), original.view());
    EXPECT_EQ(copy.view(), "data");
}

TEST_F(StringOwnConstructTest, CopyConstruct_Empty_IsEmpty) {
    const StringOwn original{};
    const StringOwn copy{ original };
    EXPECT_TRUE(copy.view().empty());
}

TEST_F(StringOwnConstructTest, MoveConstruct_TransfersOwnership) {
    StringOwn original{ std::string("move") };
    const StringOwn moved{ std::move(original) };
    EXPECT_EQ(moved.view(), "move");
}

TEST_F(StringOwnConstructTest, CopyConstruct_MutatingCopyDoesNotAffectOriginal) {
    const StringOwn original{ std::string("original") };
    StringOwn copy{ original };
    copy = std::string("changed");
    EXPECT_EQ(original.view(), "original");
    EXPECT_EQ(copy.view(), "changed");
}

#pragma endregion


#pragma region Conversion Methods

struct StringOwnConversionTest : testing::Test {};

TEST_F(StringOwnConversionTest, ToStringView) {
    const StringOwn own{ "abc" };
    const std::string_view sv{ own };
    EXPECT_EQ(sv, "abc");
}

TEST_F(StringOwnConversionTest, ToString) {
    const StringOwn own{ "abc" };
    const std::string s{ own };
    EXPECT_EQ(s, "abc");
}

TEST_F(StringOwnConversionTest, View_ReturnsStringView) {
    const StringOwn own{ "abc" };
    EXPECT_EQ(own.view(), "abc");
}

TEST_F(StringOwnConversionTest, View_EmptyOwn_ReturnsEmptyView) {
    const StringOwn own{};
    EXPECT_TRUE(own.view().empty());
}

TEST_F(StringOwnConversionTest, String_ConstRef_ReturnsRefToString) {
    const StringOwn own{ "abc" };
    EXPECT_EQ(own.string(), "abc");
}

TEST_F(StringOwnConversionTest, String_MutableRef_ReturnsMutableRef) {
    StringOwn own{ "abc" };
    own.string() = "xyz";
    EXPECT_EQ(own.view(), "xyz");
}

TEST_F(StringOwnConversionTest, String_EmptyOwn_ReturnsEmptyString) {
    const StringOwn own{};
    EXPECT_TRUE(own.string().empty());
}

#pragma endregion


#pragma region Range Interface

struct StringOwnRangeTest : testing::Test {
    const StringOwn own{ std::string("hello") };
};

TEST_F(StringOwnRangeTest, RangeFor_IteratesCharsInOrder) {
    std::string collected;
    for (const auto c : own)
        collected += c;
    EXPECT_EQ(collected, "hello");
}

TEST_F(StringOwnRangeTest, BeginEnd_MatchStringViewIterators) {
    EXPECT_EQ(own.begin(), own.view().begin());
    EXPECT_EQ(own.end(), own.view().end());
}

TEST_F(StringOwnRangeTest, CBeginCEnd_ConstIteration) {
    std::string collected;
    for (auto it{ own.cbegin() }; it != own.cend(); ++it)
        collected += *it;
    EXPECT_EQ(collected, "hello");
}

TEST_F(StringOwnRangeTest, Size_ReturnsLength) {
    EXPECT_EQ(own.size(), 5u);
}

TEST_F(StringOwnRangeTest, Empty_FalseForNonEmpty) {
    EXPECT_FALSE(own.empty());
}

TEST_F(StringOwnRangeTest, Empty_TrueForDefaultConstructed) {
    const StringOwn empty{};
    EXPECT_TRUE(empty.empty());
}

TEST_F(StringOwnRangeTest, Data_ReturnsPointerToChars) {
    EXPECT_EQ((std::string_view{ own.data(), own.size() }), "hello");
}

TEST_F(StringOwnRangeTest, Front_ReturnsFirstChar) {
    EXPECT_EQ(own.front(), 'h');
}

TEST_F(StringOwnRangeTest, Back_ReturnsLastChar) {
    EXPECT_EQ(own.back(), 'o');
}

TEST_F(StringOwnRangeTest, IndexAccess_Forward) {
    EXPECT_EQ(own[0], 'h');
    EXPECT_EQ(own[2], 'l');
    EXPECT_EQ(own[4], 'o');
}

TEST_F(StringOwnRangeTest, Contains_Substring_True) {
    EXPECT_TRUE(own.contains(std::string_view{ "ell" }));
    EXPECT_TRUE(own.contains("ell"));
}

TEST_F(StringOwnRangeTest, Contains_Substring_False) {
    EXPECT_FALSE(own.contains(std::string_view{ "xyz" }));
    EXPECT_FALSE(own.contains("xyz"));
}

TEST_F(StringOwnRangeTest, Contains_Char_True) {
    EXPECT_TRUE(own.contains('h'));
    EXPECT_TRUE(own.contains('o'));
}

TEST_F(StringOwnRangeTest, Contains_Char_False) {
    EXPECT_FALSE(own.contains('z'));
}

TEST_F(StringOwnRangeTest, ReverseIterators_ReversedOrder) {
    std::string collected;
    for (auto it{ own.rbegin() }; it != own.rend(); ++it)
        collected += *it;
    EXPECT_EQ(collected, "olleh");
}

TEST_F(StringOwnRangeTest, RangesDistance_MatchesSize) {
    EXPECT_EQ(std::ranges::distance(own), 5);
}

TEST_F(StringOwnRangeTest, RangesFilterView_YieldsVowels) {
    auto vowels{ own | std::views::as_const | std::views::filter([](char c) {
        return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u';
    }) };
    std::string collected;
    for (const auto c : vowels)
        collected += c;
    EXPECT_EQ(collected, "eo");
}

TEST_F(StringOwnRangeTest, RangesTransformView_UpperCase) {
    auto upper{ own | std::views::transform([](char c) {
        return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }) };
    std::string collected;
    for (const auto c : upper)
        collected += c;
    EXPECT_EQ(collected, "HELLO");
}

TEST_F(StringOwnRangeTest, RangeConcepts) {
    static_assert(std::ranges::range<StringOwn>);
    static_assert(std::ranges::sized_range<StringOwn>);
    static_assert(std::ranges::common_range<StringOwn>);
}

#pragma endregion


#pragma region Equality

struct StringOwnEqualityTest : testing::Test {};

TEST_F(StringOwnEqualityTest, SameContent_Equal) {
    const StringOwn a{ "hello" };
    const StringOwn b{ "hello" };
    EXPECT_EQ(a, b);
}

TEST_F(StringOwnEqualityTest, DifferentContent_NotEqual) {
    const StringOwn a{ "hello" };
    const StringOwn b{ "world" };
    EXPECT_NE(a, b);
}

TEST_F(StringOwnEqualityTest, EmptyAndEmpty_Equal) {
    const StringOwn a{};
    const StringOwn b{};
    EXPECT_EQ(a, b);
}

TEST_F(StringOwnEqualityTest, EmptyAndNonEmpty_NotEqual) {
    const StringOwn a{};
    const StringOwn b{ "x" };
    EXPECT_NE(a, b);
}

TEST_F(StringOwnEqualityTest, CompareWithStringView_Equal) {
    const StringOwn own{ "test" };
    EXPECT_TRUE(own == std::string_view{ "test" });
}

TEST_F(StringOwnEqualityTest, CompareWithStringView_NotEqual) {
    const StringOwn own{ "test" };
    EXPECT_FALSE(own == std::string_view{ "other" });
}

TEST_F(StringOwnEqualityTest, CompareWithStringView_Empty) {
    const StringOwn own{};
    EXPECT_TRUE(own == std::string_view{});
}

#pragma endregion


#pragma region Formatting

struct StringOwnFormatTest : testing::Test {};

TEST_F(StringOwnFormatTest, Format_PrintsContent) {
    const StringOwn own{ "hello" };
    EXPECT_EQ(std::format("{}", own), "hello");
}

TEST_F(StringOwnFormatTest, Format_Empty) {
    const StringOwn own{};
    EXPECT_EQ(std::format("{}", own), "");
}

TEST_F(StringOwnFormatTest, Format_WithSpecialChars) {
    const StringOwn own{ "tab\there" };
    EXPECT_EQ(std::format("{}", own), "tab\there");
}

#pragma endregion


#pragma region Assignment

struct StringOwnAssignTest : testing::Test {};

TEST_F(StringOwnAssignTest, AssignFromStdString_UpdatesContent) {
    StringOwn own{ "old" };
    own = std::string{ "new" };
    EXPECT_EQ(own.view(), "new");
}

TEST_F(StringOwnAssignTest, AssignFromStringView_UpdatesContent) {
    StringOwn own{ "old" };
    own = std::string_view{ "new" };
    EXPECT_EQ(own.view(), "new");
}

TEST_F(StringOwnAssignTest, AssignFromOwn_UpdatesContent) {
    StringOwn own{ "old" };
    own = StringOwn{ std::string("new") };
    EXPECT_EQ(own.view(), "new");
}

TEST_F(StringOwnAssignTest, AssignFromRvalue_UsesMove) {
    StringOwn own{ "old" };
    own = std::string{ "rvalue" };
    EXPECT_EQ(own.view(), "rvalue");
}

TEST_F(StringOwnAssignTest, SelfAssignment_DoesNotCorrupt) {
    StringOwn own{ "self" };
    own = own;
    EXPECT_EQ(own.view(), "self");
}

#pragma endregion


#pragma region Cross-Type Interactions

struct StringOwnRefTest : testing::Test {
    const std::string backing{ "shared" };
    const StringRef ref{ backing };
    const StringOwn own{ ref };
};

TEST_F(StringOwnRefTest, FromStringRef_OwnsIndependentCopy) {
    EXPECT_EQ(own.view(), "shared");
    // Modifying the backing store must not affect StringOwn
    // (StringOwn made its own allocation)
    EXPECT_NE(own.data(), ref.data());
}

TEST_F(StringOwnRefTest, StringOwnAndRefWithSameContent_Equal) {
    const StringOwn own2{ "shared" };
    EXPECT_EQ(own.view(), own2.view());
}

#pragma endregion

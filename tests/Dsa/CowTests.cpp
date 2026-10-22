#include <gtest/gtest.h>
#include <Thoth/Dsa/Cow.hpp>
#include <Thoth/NJson/StringRef.hpp>
#include <Thoth/NJson/StringOwn.hpp>
#include <Hermes/Utils/Overloads.hpp>

#include <algorithm>
#include <cctype>
#include <functional>
#include <format>
#include <ranges>
#include <string>
#include <string_view>

using namespace Thoth::Dsa;
using namespace Thoth::NJson;

using StringCow = Cow<StringRef, std::string>;
using ThothString = Cow<StringRef, StringOwn>;


#pragma region Construction

struct CowConstructTest : testing::Test {};

TEST_F(CowConstructTest, DefaultConstruct_IsRef) {
    StringCow cow{};
    EXPECT_TRUE(cow.IsRef());
}

TEST_F(CowConstructTest, FromRef_IsRef) {
    const StringCow cow{ StringCow::FromRef(StringRef{}) };
    EXPECT_TRUE(cow.IsRef());
}

TEST_F(CowConstructTest, FromOwned_IsNotRef) {
    const StringCow cow{ StringCow::FromOwned(std::string("hello")) };
    EXPECT_FALSE(cow.IsRef());
}

TEST_F(CowConstructTest, FromOwned_Rvalue_IsNotRef) {
    StringCow cow{ StringCow::FromOwned(std::string("world")) };
    EXPECT_FALSE(cow.IsRef());
}

TEST_F(CowConstructTest, IsRefType_StaticCheck) {
    const StringCow ref{ StringCow::FromRef(StringRef{}) };
    EXPECT_TRUE(StringCow::IsRefType(ref));

    const StringCow own{ StringCow::FromOwned(std::string("x")) };
    EXPECT_FALSE(StringCow::IsRefType(own));
}

TEST_F(CowConstructTest, CopyConstruct_FromOwned_IsOwned) {
    const StringCow orig{ StringCow::FromOwned(std::string("data")) };
    const StringCow copy{ orig };
    EXPECT_FALSE(copy.IsRef());
}

#pragma endregion


#pragma region AsCopy / AsRef

struct CowValueTest : testing::Test {};

TEST_F(CowValueTest, AsCopy_FromOwned_ReturnsOwned) {
    const StringCow cow{ StringCow::FromOwned(std::string("hello")) };
    const std::string copy{ cow.AsCopy() };
    EXPECT_EQ(copy, "hello");
}

TEST_F(CowValueTest, AsCopy_DoesNotMutate_IsRefStaysRef) {
    const StringCow cow{ StringCow::FromRef(StringRef{}) };
    (void)cow.AsCopy();
    EXPECT_TRUE(cow.IsRef());  // AsCopy must not promote to Owned
}

TEST_F(CowValueTest, AsOwned_ConvertsRefToOwned) {
    StringCow cow{ StringCow::FromRef(StringRef{}) };
    EXPECT_TRUE(cow.IsRef());
    cow.AsOwned();
    EXPECT_FALSE(cow.IsRef());
}

TEST_F(CowValueTest, AsOwned_OnAlreadyOwned_StaysOwned) {
    StringCow cow{ StringCow::FromOwned(std::string("data")) };
    auto& ref{ cow.AsOwned() };
    EXPECT_EQ(ref, "data");
    EXPECT_FALSE(cow.IsRef());
}

#pragma endregion


#pragma region SetRef / SetOwned

struct CowSetTest : testing::Test {};

TEST_F(CowSetTest, SetRef_TransitionsToRef) {
    StringCow cow{ StringCow::FromOwned(std::string("was owned")) };
    EXPECT_FALSE(cow.IsRef());
    cow.SetRef(StringRef{});
    EXPECT_TRUE(cow.IsRef());
}

TEST_F(CowSetTest, SetOwned_TransitionsToOwned) {
    StringCow cow{ StringCow::FromRef(StringRef{}) };
    EXPECT_TRUE(cow.IsRef());
    cow.SetOwned(std::string("now owned"));
    EXPECT_FALSE(cow.IsRef());
}

TEST_F(CowSetTest, SetOwned_Rvalue_TransitionsToOwned) {
    StringCow cow{};
    cow.SetOwned(std::string("rvalue"));
    EXPECT_FALSE(cow.IsRef());
}

#pragma endregion


#pragma region Equality

struct CowEqualityTest : testing::Test {};

TEST_F(CowEqualityTest, TwoOwned_SameValue_Equal) {
    const StringCow a{ StringCow::FromOwned(std::string("x")) };
    const StringCow b{ StringCow::FromOwned(std::string("x")) };
    EXPECT_EQ(a, b);
}

TEST_F(CowEqualityTest, TwoOwned_DifferentValue_NotEqual) {
    const StringCow a{ StringCow::FromOwned(std::string("x")) };
    const StringCow b{ StringCow::FromOwned(std::string("y")) };
    EXPECT_NE(a, b);
}

#pragma endregion


#pragma region Visit

struct CowVisitTest : testing::Test {};

TEST_F(CowVisitTest, Visit_RefVariant_CallsRefHandler) {
    StringCow cow{ StringCow::FromRef(StringRef{}) };
    bool wasRef{ false };
    cow.Visit(Hermes::Utils::Overloaded{
        [&](StringRef)        { wasRef = true; },
        [&](std::string&)     { wasRef = false; }
    });
    EXPECT_TRUE(wasRef);
}

TEST_F(CowVisitTest, Visit_OwnedVariant_CallsOwnedHandler) {
    StringCow cow{ StringCow::FromOwned(std::string("data")) };
    bool wasOwned{ false };
    cow.Visit(Hermes::Utils::Overloaded{
        [&](StringRef)        { wasOwned = false; },
        [&](std::string&)     { wasOwned = true; }
    });
    EXPECT_TRUE(wasOwned);
}

TEST_F(CowVisitTest, Visit_Const_RefVariant) {
    const StringCow cow{ StringCow::FromRef(StringRef{}) };
    bool wasRef{ false };
    cow.Visit(Hermes::Utils::Overloaded{
        [&](const StringRef&)  { wasRef = true; },
        [&](const std::string&){ wasRef = false; }
    });
    EXPECT_TRUE(wasRef);
}

#pragma endregion


#pragma region String (Cow<StringRef, StringOwn>)

struct ThothStringTest : testing::Test {
    const std::string backing{ "hello" };
    const StringRef ref{ backing };
};

TEST_F(ThothStringTest, DefaultConstruct_IsRef) {
    ThothString cow{};
    EXPECT_TRUE(cow.IsRef());
}

TEST_F(ThothStringTest, FromRef_IsRef) {
    const ThothString cow{ ThothString::FromRef(ref) };
    EXPECT_TRUE(cow.IsRef());
}

TEST_F(ThothStringTest, FromOwned_IsNotRef) {
    const ThothString cow{ ThothString::FromOwned(StringOwn{ "hello" }) };
    EXPECT_FALSE(cow.IsRef());
}

TEST_F(ThothStringTest, FromOwned_Rvalue_IsNotRef) {
    const ThothString cow{ ThothString::FromOwned(StringOwn{ "world" }) };
    EXPECT_FALSE(cow.IsRef());
}

TEST_F(ThothStringTest, CopyConstruct_FromOwned_IsOwned) {
    const ThothString orig{ ThothString::FromOwned(StringOwn{ "data" }) };
    const ThothString copy{ orig };
    EXPECT_FALSE(copy.IsRef());
}

TEST_F(ThothStringTest, CopyConstruct_FromRef_IsRef) {
    const ThothString orig{ ThothString::FromRef(ref) };
    const ThothString copy{ orig };
    EXPECT_TRUE(copy.IsRef());
}

TEST_F(ThothStringTest, AsCopy_FromOwned_ReturnsOwnedCopy) {
    const ThothString cow{ ThothString::FromOwned(StringOwn{ "hello" }) };
    const StringOwn copy{ cow.AsCopy() };
    EXPECT_EQ(copy.view(), "hello");
    EXPECT_FALSE(cow.IsRef());
}

TEST_F(ThothStringTest, AsCopy_FromRef_DoesNotPromoteToOwned) {
    const ThothString cow{ ThothString::FromRef(ref) };
    (void)cow.AsCopy();
    EXPECT_TRUE(cow.IsRef());
}

TEST_F(ThothStringTest, AsOwned_ConvertsRefToOwned) {
    ThothString cow{ ThothString::FromRef(ref) };
    EXPECT_TRUE(cow.IsRef());
    cow.AsOwned();
    EXPECT_FALSE(cow.IsRef());
}

TEST_F(ThothStringTest, AsOwned_OnAlreadyOwned_StaysOwned) {
    ThothString cow{ ThothString::FromOwned(StringOwn{ "data" }) };
    auto& owned{ cow.AsOwned() };
    EXPECT_EQ(owned.view(), "data");
    EXPECT_FALSE(cow.IsRef());
}

TEST_F(ThothStringTest, AsRef_FromOwned_ReturnsRef) {
    const ThothString cow{ ThothString::FromOwned(StringOwn{ "data" }) };
    const StringRef r{ cow.AsRef() };
    EXPECT_EQ(r.AsStringView(), "data");
}

TEST_F(ThothStringTest, SetRef_TransitionsToRef) {
    ThothString cow{ ThothString::FromOwned(StringOwn{ "was owned" }) };
    EXPECT_FALSE(cow.IsRef());
    cow.SetRef(ref);
    EXPECT_TRUE(cow.IsRef());
}

TEST_F(ThothStringTest, SetOwned_TransitionsToOwned) {
    ThothString cow{ ThothString::FromRef(ref) };
    EXPECT_TRUE(cow.IsRef());
    cow.SetOwned(StringOwn{ "now owned" });
    EXPECT_FALSE(cow.IsRef());
}

TEST_F(ThothStringTest, Equality_TwoOwnedSameContent_Equal) {
    const ThothString a{ ThothString::FromOwned(StringOwn{ "x" }) };
    const ThothString b{ ThothString::FromOwned(StringOwn{ "x" }) };
    EXPECT_EQ(a, b);
}

TEST_F(ThothStringTest, Equality_TwoOwnedDifferentContent_NotEqual) {
    const ThothString a{ ThothString::FromOwned(StringOwn{ "x" }) };
    const ThothString b{ ThothString::FromOwned(StringOwn{ "y" }) };
    EXPECT_NE(a, b);
}

TEST_F(ThothStringTest, Equality_RefAndOwnedSameContent_Equal) {
    const ThothString a{ ThothString::FromRef(ref) };
    const ThothString b{ ThothString::FromOwned(StringOwn{ "hello" }) };
    EXPECT_EQ(a, b);
}

TEST_F(ThothStringTest, Equality_TwoRefSameContent_Equal) {
    const std::string s1{ "abc" };
    const std::string s2{ "abc" };
    const StringRef r1{ s1 };
    const StringRef r2{ s2 };
    const ThothString a{ ThothString::FromRef(r1) };
    const ThothString b{ ThothString::FromRef(r2) };
    EXPECT_EQ(a, b);
}

TEST_F(ThothStringTest, ImplicitConversion_ToRef) {
    const ThothString cow{ ThothString::FromOwned(StringOwn{ "data" }) };
    const StringRef r{ cow };
    EXPECT_EQ(r.AsStringView(), "data");
}

TEST_F(ThothStringTest, Visit_RefVariant_CallsRefHandler) {
    const ThothString cow{ ThothString::FromRef(ref) };
    bool wasRef{ false };
    cow.Visit(Hermes::Utils::Overloaded{
        [&](const StringRef)       { wasRef = true; },
        [&](const StringOwn&)      { wasRef = false; }
    });
    EXPECT_TRUE(wasRef);
}

TEST_F(ThothStringTest, Visit_OwnedVariant_CallsOwnedHandler) {
    ThothString cow{ ThothString::FromOwned(StringOwn{ "data" }) };
    bool wasOwned{ false };
    cow.Visit(Hermes::Utils::Overloaded{
        [&](StringRef)       { wasOwned = false; },
        [&](StringOwn&)      { wasOwned = true; }
    });
    EXPECT_TRUE(wasOwned);
}

TEST_F(ThothStringTest, Visit_Const_OwnedVariant) {
    const ThothString cow{ ThothString::FromOwned(StringOwn{ "data" }) };
    bool wasOwned{ false };
    cow.Visit(Hermes::Utils::Overloaded{
        [&](const StringRef&)  { wasOwned = false; },
        [&](const StringOwn&)  { wasOwned = true; }
    });
    EXPECT_TRUE(wasOwned);
}

TEST_F(ThothStringTest, Format_PrintsOwnedContent) {
    const ThothString cow{ ThothString::FromOwned(StringOwn{ "hello" }) };
    EXPECT_EQ(std::format("{}", cow), "hello");
}

TEST_F(ThothStringTest, Format_PrintsRefContent) {
    const ThothString cow{ ThothString::FromRef(ref) };
    EXPECT_EQ(std::format("{}", cow), "hello");
}

TEST_F(ThothStringTest, Format_EmptyRef) {
    const ThothString cow{};
    EXPECT_EQ(std::format("{}", cow), "");
}

#pragma endregion


#pragma region Range Interface (Cow)

struct CowRangeTest : testing::Test {
    const std::string backing{ "hello" };
    const StringRef ref{ backing };
    const ThothString thothCow{ ThothString::FromOwned(StringOwn{ "world" }) };
};

TEST_F(CowRangeTest, StringCow_RangeFor_IteratesChars_Ref) {
    const StringCow cow{ StringCow::FromRef(ref) };
    std::string collected;
    for (const auto c : cow)
        collected += c;
    EXPECT_EQ(collected, "hello");
}

TEST_F(CowRangeTest, StringCow_RangeFor_IteratesChars_Owned) {
    const StringCow cow{ StringCow::FromOwned(std::string("world")) };
    std::string collected;
    for (const auto c : cow)
        collected += c;
    EXPECT_EQ(collected, "world");
}

TEST_F(CowRangeTest, ThothString_RangeFor_IteratesChars_Ref) {
    const ThothString cow{ ThothString::FromRef(ref) };
    std::string collected;
    for (const auto c : cow)
        collected += c;
    EXPECT_EQ(collected, "hello");
}

TEST_F(CowRangeTest, ThothString_RangeFor_IteratesChars_Owned) {
    std::string collected;
    for (const auto c : thothCow)
        collected += c;
    EXPECT_EQ(collected, "world");
}

TEST_F(CowRangeTest, ThothString_BeginEnd_MatchView) {
    const ThothString cow{ ThothString::FromOwned(StringOwn{ "hello" }) };
    const StringRef r{ cow.AsRef() };
    EXPECT_EQ(cow.begin(), r.begin());
    EXPECT_EQ(cow.end(), r.end());
}

TEST_F(CowRangeTest, ThothString_Size_ReturnsLength) {
    EXPECT_EQ(thothCow.size(), 5u);
}

TEST_F(CowRangeTest, ThothString_Empty_FalseForNonEmpty) {
    EXPECT_FALSE(thothCow.empty());
}

TEST_F(CowRangeTest, ThothCow_Empty_TrueForDefault) {
    const ThothString cow{};
    EXPECT_TRUE(cow.empty());
}

TEST_F(CowRangeTest, ThothString_Front_Back) {
    EXPECT_EQ(thothCow.front(), 'w');
    EXPECT_EQ(thothCow.back(), 'd');
}

TEST_F(CowRangeTest, ThothString_Data_ReturnsPtr) {
    EXPECT_EQ((std::string_view{ thothCow.data(), thothCow.size() }), "world");
}

TEST_F(CowRangeTest, ThothString_IndexAccess) {
    EXPECT_EQ(thothCow[0], 'w');
    EXPECT_EQ(thothCow[4], 'd');
}

TEST_F(CowRangeTest, ThothString_Contains_Substring) {
    EXPECT_TRUE(thothCow.contains(std::string_view{ "orl" }));
    EXPECT_FALSE(thothCow.contains(std::string_view{ "xyz" }));
}

TEST_F(CowRangeTest, ThothString_Contains_Char) {
    EXPECT_TRUE(thothCow.contains('w'));
    EXPECT_TRUE(thothCow.contains('d'));
    EXPECT_FALSE(thothCow.contains('z'));
}

TEST_F(CowRangeTest, ThothString_ReverseIterators) {
    std::string collected;
    for (auto it{ thothCow.rbegin() }; it != thothCow.rend(); ++it)
        collected += *it;
    EXPECT_EQ(collected, "dlrow");
}

TEST_F(CowRangeTest, ThothString_RangesDistance) {
    EXPECT_EQ(std::ranges::distance(thothCow), 5);
}

TEST_F(CowRangeTest, ThothString_RangesTransform) {
    const auto upper{ thothCow | std::views::transform([](char c) {
        return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }) };
    std::string collected;
    for (const auto c : upper)
        collected += c;
    EXPECT_EQ(collected, "WORLD");
}

TEST_F(CowRangeTest, RangeConcepts) {
    static_assert(std::ranges::range<ThothString>);
    static_assert(std::ranges::sized_range<ThothString>);
    static_assert(std::ranges::common_range<ThothString>);
}

#pragma endregion
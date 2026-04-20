#include <gtest/gtest.h>
#include <Thoth/Dsa/Cow.hpp>
#include <Thoth/Utils/Overloads.hpp>
#include <Thoth/NJson/StringRef.hpp>


using namespace Thoth::Dsa;
using namespace Thoth::NJson;

using StringCow = Cow<StringRef, std::string>;


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
    cow.Visit(Thoth::Utils::Overloaded{
        [&](StringRef)        { wasRef = true; },
        [&](std::string&)     { wasRef = false; }
    });
    EXPECT_TRUE(wasRef);
}

TEST_F(CowVisitTest, Visit_OwnedVariant_CallsOwnedHandler) {
    StringCow cow{ StringCow::FromOwned(std::string("data")) };
    bool wasOwned{ false };
    cow.Visit(Thoth::Utils::Overloaded{
        [&](StringRef)        { wasOwned = false; },
        [&](std::string&)     { wasOwned = true; }
    });
    EXPECT_TRUE(wasOwned);
}

TEST_F(CowVisitTest, Visit_Const_RefVariant) {
    const StringCow cow{ StringCow::FromRef(StringRef{}) };
    bool wasRef{ false };
    cow.Visit(Thoth::Utils::Overloaded{
        [&](const StringRef&)  { wasRef = true; },
        [&](const std::string&){ wasRef = false; }
    });
    EXPECT_TRUE(wasRef);
}

#pragma endregion
#include <gtest/gtest.h>
#include <Thoth/Utils/Monostate.hpp>
#include <Thoth/Utils/LastMatchVariant.hpp>
#include <Thoth/Utils/Ranges/SharedInputView.hpp>

#include <variant>
#include <ranges>
#include <string>

using namespace Thoth::Utils;


#pragma region Monostate formatter

struct MonostateFormatTest : testing::Test {};

TEST_F(MonostateFormatTest, Format_Monostate_PrintsNull) {
    EXPECT_EQ(std::format("{}", std::monostate{}), "null");
}

TEST_F(MonostateFormatTest, Format_MonostateInVariant_PrintsNull) {
    const std::variant<std::monostate, int> v{ std::monostate{} };
    const auto result{ std::visit([](auto x) { return std::format("{}", x); }, v) };
    EXPECT_EQ(result, "null");
}

#pragma endregion


#pragma region LastMatchVariant - FindMatchIndex

struct FindMatchIndexTest : testing::Test {
    using Var = std::variant<int, double, std::string>;
};

TEST_F(FindMatchIndexTest, SameType_Int_ReturnsIndex0) {
    constexpr auto idx{ FindMatchIndex<int, Var, std::is_same>() };
    EXPECT_EQ(idx, 0u);
}

TEST_F(FindMatchIndexTest, SameType_Double_ReturnsIndex1) {
    constexpr auto idx{ FindMatchIndex<double, Var, std::is_same>() };
    EXPECT_EQ(idx, 1u);
}

TEST_F(FindMatchIndexTest, SameType_String_ReturnsIndex2) {
    constexpr auto idx{ FindMatchIndex<std::string, Var, std::is_same>() };
    EXPECT_EQ(idx, 2u);
}

TEST_F(FindMatchIndexTest, Convertible_Int_ToDouble_FindsDouble) {
    constexpr auto idx{ FindMatchIndex<int, Var, std::is_convertible>() };
    // int is convertible to int (index 0) - first match
    EXPECT_EQ(idx, 0u);
}

#pragma endregion


#pragma region LastMatchVariant - FirstConvertibleVariant / FirstEqualVariant

struct FirstVariantTest : testing::Test {
    using Var = std::variant<int, double, std::string>;
};

TEST_F(FirstVariantTest, FirstEqualVariant_Int_IsInt) {
    using Result = FirstEqualVariant<int, Var>;
    EXPECT_TRUE((std::is_same_v<Result, int>));
}

TEST_F(FirstVariantTest, FirstEqualVariant_String_IsString) {
    using Result = FirstEqualVariant<std::string, Var>;
    EXPECT_TRUE((std::is_same_v<Result, std::string>));
}

TEST_F(FirstVariantTest, FirstConvertibleVariant_Int_IsInt) {
    using Result = FirstConvertibleVariant<int, Var>;
    EXPECT_TRUE((std::is_same_v<Result, int>));
}

#pragma endregion


#pragma region SharedInputView

struct SharedInputViewTest : testing::Test {};

TEST_F(SharedInputViewTest, IotaRange_IteratesCorrectly) {
    auto iota{ std::views::iota(0, 5) };
    SharedInputView view{ std::move(iota) };

    std::vector<int> result;
    for (auto it{ view.begin() }; it != view.end(); ++it)
        result.push_back(*it);

    EXPECT_EQ(result, (std::vector<int>{ 0, 1, 2, 3, 4 }));
}

TEST_F(SharedInputViewTest, EmptyRange_BeginEqualsEnd) {
    auto iota{ std::views::iota(0, 0) };
    SharedInputView view{ std::move(iota) };
    EXPECT_EQ(view.begin(), view.end());
}

TEST_F(SharedInputViewTest, SingleElement_IteratesOnce) {
    auto iota{ std::views::iota(7, 8) };
    SharedInputView view{ std::move(iota) };
    auto it{ view.begin() };
    EXPECT_EQ(*it, 7);
    ++it;
    EXPECT_EQ(it, view.end());
}

TEST_F(SharedInputViewTest, SatisfiesRangeConcept) {
    auto iota{ std::views::iota(0, 3) };
    SharedInputView<decltype(iota)> view{ std::move(iota) };
    EXPECT_TRUE((std::ranges::range<decltype(view)>));
}

#pragma endregion
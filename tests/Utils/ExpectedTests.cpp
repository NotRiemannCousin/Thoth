#include <gtest/gtest.h>
#include <Thoth/Utils/Expected.hpp>

#include <string>
#include <optional>
#include <expected>

#pragma region ValueOr

using namespace Thoth::Utils;

namespace {
    struct ValueOrTest : testing::Test {};
}

TEST_F(ValueOrTest, HasValue_ReturnsValue) {
    std::optional<int> opt{ 42 };
    const auto result{ ValueOr(std::move(opt), std::string{ "err" }) };
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 42);
}

TEST_F(ValueOrTest, NoValue_ReturnsUnexpected) {
    std::optional<int> opt{};
    const auto result{ ValueOr(std::move(opt), std::string{ "err" }) };
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error(), "err");
}

#pragma endregion


#pragma region ErrorIf / ErrorIfNot

struct ErrorIfTest : testing::Test {};

TEST_F(ErrorIfTest, ErrorIf_PredicateFalse_ReturnsValue) {
    const auto result{ ErrorIf([](int x) { return x < 0; }, 5, std::string("negative")) };
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 5);
}

TEST_F(ErrorIfTest, ErrorIf_PredicateTrue_ReturnsUnexpected) {
    const auto result{ ErrorIf([](int x) { return x < 0; }, -1, std::string("negative")) };
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error(), "negative");
}

TEST_F(ErrorIfTest, ErrorIfNot_PredicateTrue_ReturnsValue) {
    const auto result{ ErrorIfNot([](int x) { return x > 0; }, 5, std::string("not positive")) };
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 5);
}

TEST_F(ErrorIfTest, ErrorIfNot_PredicateFalse_ReturnsUnexpected) {
    const auto result{ ErrorIfNot([](int x) { return x > 0; }, -1, std::string("not positive")) };
    EXPECT_FALSE(result);
}

TEST_F(ErrorIfTest, H_ErrorIf_ReturnsCallable) {
    auto check{ H_ErrorIf([](int x) { return x == 0; }, std::string("zero")) };
    EXPECT_FALSE(check(0));
    ASSERT_TRUE(check(1));
    EXPECT_EQ(*check(1), 1);
}

TEST_F(ErrorIfTest, H_ErrorIfNot_ReturnsCallable) {
    auto check{ H_ErrorIfNot([](int x) { return x > 0; }, std::string("bad")) };
    EXPECT_FALSE(check(-1));
    ASSERT_TRUE(check(10));
}

#pragma endregion


#pragma region CallIfError / CallIfErrorNot

struct CallIfErrorTest : testing::Test {};

TEST_F(CallIfErrorTest, CallIfError_PredicateTrue_ReturnsValue) {
    const auto result{ CallIfError(
        [](const std::string& s) { return !s.empty(); },
        [](const std::string& s) { return static_cast<int>(s.size()); },
        std::string("hello")
    ) };
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, std::string("hello"));
}

TEST_F(CallIfErrorTest, CallIfError_PredicateFalse_ReturnsTransformed) {
    const auto result{ CallIfError(
        [](const std::string& s) { return !s.empty(); },
        [](const std::string& s) { return static_cast<int>(s.size()); },
        std::string("")
    ) };
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error(), 0);
}

TEST_F(CallIfErrorTest, H_CallIfError_ReturnsCallable) {
    auto fn{ H_CallIfError(
        [](int x) { return x > 0; },
        [](int x) { return std::string("err: ") + std::to_string(x); }
    ) };
    ASSERT_TRUE(fn(5));
    EXPECT_FALSE(fn(-1));
}

#pragma endregion


#pragma region FoldWhileSuccess

struct FoldWhileSuccessTest : testing::Test {};

TEST_F(FoldWhileSuccessTest, AllSuccess_ReturnsFoldedValue) {
    std::vector<std::expected<int, std::string>> values{
        std::expected<int, std::string>{ 1 },
        std::expected<int, std::string>{ 2 },
        std::expected<int, std::string>{ 3 }
    };
    const auto result{ FoldWhileSuccess(values, [](int acc, int v) { return acc + v; }, 0) };
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 6);
}

TEST_F(FoldWhileSuccessTest, ErrorInMiddle_ReturnsError) {
    std::vector<std::expected<int, std::string>> values{
        std::expected<int, std::string>{ 1 },
        std::expected<int, std::string>{ std::unexpected(std::string("fail")) },
        std::expected<int, std::string>{ 3 }
    };
    const auto result{ FoldWhileSuccess(values, [](int acc, int v) { return acc + v; }, 0) };
    EXPECT_FALSE(result);
    EXPECT_EQ(result.error(), "fail");
}

TEST_F(FoldWhileSuccessTest, EmptyRange_ReturnsInitial) {
    std::vector<std::expected<int, std::string>> values{};
    const auto result{ FoldWhileSuccess(values, [](int acc, int v) { return acc + v; }, 99) };
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 99);
}

#pragma endregion


#pragma region ToValue

struct ToValueTest : testing::Test {};

TEST_F(ToValueTest, ToValue_Integral_ReturnsValue) {
    constexpr auto v{ ToValue<42>() };
    EXPECT_EQ(v, 42);
}

TEST_F(ToValueTest, ToValue_Type_DefaultConstructs) {
    const auto v{ ToValue<std::string>() };
    EXPECT_EQ(v, "");
}

#pragma endregion


#include <gtest/gtest.h>
#include <Thoth/Utils/Optional.hpp>

#include <string>
#include <optional>
#include <expected>

using namespace Thoth::Utils;

#pragma region TransformOptIf / TransformOptIfNot

namespace {
    struct TransformOptIfTest : testing::Test {};
}

TEST_F(TransformOptIfTest, TransformOptIf_PredicateTrue_ReturnsTransformed) {
    const auto result{ TransformOptIf(
        [](int x) { return x > 0; },
        [](int x) { return x * 2; },
        5
    ) };
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, 10);
}

TEST_F(TransformOptIfTest, TransformOptIf_PredicateFalse_ReturnsNullopt) {
    const auto result{ TransformOptIf(
        [](int x) { return x > 0; },
        [](int x) { return x * 2; },
        -1
    ) };
    EXPECT_FALSE(result);
}

TEST_F(TransformOptIfTest, TransformOptIfNot_PredicateFalse_ReturnsTransformed) {
    const auto result{ TransformOptIfNot(
        [](int x) { return x < 0; },
        [](int x) { return std::to_string(x); },
        5
    ) };
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "5");
}

TEST_F(TransformOptIfTest, TransformOptIfNot_PredicateTrue_ReturnsNullopt) {
    const auto result{ TransformOptIfNot(
        [](int x) { return x < 0; },
        [](int x) { return std::to_string(x); },
        -3
    ) };
    EXPECT_FALSE(result);
}

TEST_F(TransformOptIfTest, H_TransformOptIf_ReturnsCallable) {
    auto fn{ H_TransformOptIf(
        [](int x) { return x % 2 == 0; },
        [](int x) { return x / 2; }
    ) };
    ASSERT_TRUE(fn(4));
    EXPECT_EQ(*fn(4), 2);
    EXPECT_FALSE(fn(3));
}

#pragma endregion

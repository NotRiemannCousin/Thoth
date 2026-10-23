// #include <gtest/gtest.h>
// #include <Thoth/Utils/Functional.hpp>
//
// #include <string>
// #include <optional>
// #include <expected>
//
// using namespace Thoth::Utils;
//
//
// TEST_F(SelfTest, Self_SideEffect_ValuePassedThrough) {
//     int sideEffect{};
//     const int result{ Self(42, [&](int v) { sideEffect = v; }) };
//     EXPECT_EQ(result, 42);
//     EXPECT_EQ(sideEffect, 42);
// }
//
// TEST_F(SelfTest, Self_StringPassThrough_ReturnsOriginal) {
//     std::string modified;
//     const std::string result{ Self(std::string("hello"), [&](const std::string& s) { modified = s + "!"; }) };
//     EXPECT_EQ(result, "hello");
//     EXPECT_EQ(modified, "hello!");
// }
//
// #pragma endregion
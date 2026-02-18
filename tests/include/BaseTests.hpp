#pragma once
#include <print>

// Light Yellow
#define NAME_TEST_COLOR         "\33[1;33m"

// Red
#define ERROR_TEST_COLOR        "\33[1;31m"
// Green
#define PASSED_TEST_COLOR       "\33[1;32m"

// Cyan/Light White
#define INFO_TEST_COLOR         "\33[1;37m"

// Reset
#define RESET_COLOR             "\33[0m"

using std::string_view;

inline string_view OK_MESSAGE  { PASSED_TEST_COLOR " [OK] "   RESET_COLOR };
inline string_view FAIL_MESSAGE{ ERROR_TEST_COLOR  " [FAIL] " RESET_COLOR };

inline void Test(string_view testName, bool passed) {
    std::println(NAME_TEST_COLOR "{}" RESET_COLOR " {}",
        testName, passed ? OK_MESSAGE : FAIL_MESSAGE);
}

inline void TestBattery(string_view name) {
    std::println(INFO_TEST_COLOR "\n\n{}" RESET_COLOR, name);
}

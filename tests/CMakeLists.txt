include(FetchContent)

FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(
        ThothTests

        Json/JsonTests.cpp
        Http/UrlTests.cpp
        Http/HeadersTests.cpp
        Utils/StringUtilsTests.cpp
        Dsa/LinearMapTests.cpp
        Dsa/CowTests.cpp
        String/StringRefTests.cpp
        String/UnicodeViewerTests.cpp
        Utils/FunctionalTests.cpp
        Utils/UtilsTests.cpp
        Http/TypedHeaderTests.cpp
        Dsa/FileOutputTests.cpp
)

target_include_directories(
        ThothTests
        PUBLIC
        ${CMAKE_SOURCE_DIR}/tests
)

target_link_libraries(
        ThothTests
        PRIVATE
        Thoth
        GTest::gtest_main
)

gtest_discover_tests(ThothTests)
CPMFindPackage(
        NAME              GTest
        VERSION           1.17.0
        GITHUB_REPOSITORY google/googletest
        GIT_TAG           v1.17.0
)

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
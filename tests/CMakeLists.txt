CPMFindPackage(
        NAME              GTest
        VERSION           1.17.0
        GITHUB_REPOSITORY google/googletest
        GIT_TAG           v1.17.0
)
include(GoogleTest)

function(add_thoth_test TARGET_NAME)
    add_executable(${TARGET_NAME} ${ARGN})

    target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/tests)
    target_link_libraries(${TARGET_NAME} PRIVATE Thoth GTest::gtest_main)

    gtest_discover_tests(${TARGET_NAME})
endfunction()

add_subdirectory(Json)
add_subdirectory(Http)
add_subdirectory(Dsa)
add_subdirectory(String)
add_subdirectory(Utils)
add_subdirectory(Middleware)
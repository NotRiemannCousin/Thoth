CPMFindPackage(
        NAME              GTest
        VERSION           1.17.0
        GITHUB_REPOSITORY google/googletest
        GIT_TAG           v1.17.0
)
include(GoogleTest)

add_subdirectory(Json)
add_subdirectory(Http)
add_subdirectory(Dsa)
add_subdirectory(String)
add_subdirectory(Utils)
add_subdirectory(Middleware)
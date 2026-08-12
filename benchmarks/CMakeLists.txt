# -----------------------------------------------------------------------
#  Dependencies via CPM
# -----------------------------------------------------------------------

CPMFindPackage(
        NAME benchmark
        GITHUB_REPOSITORY google/benchmark
        GIT_TAG v1.9.1
        OPTIONS "BENCHMARK_ENABLE_TESTING OFF"
        "BENCHMARK_ENABLE_GTEST_TESTS OFF"
        "BENCHMARK_ENABLE_INSTALL OFF"
)

CPMFindPackage(
        NAME nlohmann_json
        GITHUB_REPOSITORY nlohmann/json
        GIT_TAG v3.11.3
        OPTIONS "JSON_BuildTests OFF" "JSON_Install OFF"
)

CPMFindPackage(
        NAME simdjson
        GITHUB_REPOSITORY simdjson/simdjson
        GIT_TAG v3.11.4
        OPTIONS "SIMDJSON_ENABLE_THREADS OFF"
)

CPMFindPackage(
        NAME RapidJSON
        GITHUB_REPOSITORY Tencent/rapidjson
        GIT_TAG master
        OPTIONS "RAPIDJSON_BUILD_DOC OFF"
        "RAPIDJSON_BUILD_EXAMPLES OFF"
        "RAPIDJSON_BUILD_TESTS OFF"
        "RAPIDJSON_BUILD_CRT_LINKAGE ON"
)

CPMFindPackage(
        NAME httplib
        GITHUB_REPOSITORY yhirose/cpp-httplib
        GIT_TAG v0.18.3
        OPTIONS "HTTPLIB_COMPILE ON"
        "HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF"
        "HTTPLIB_INSTALL OFF"
)

set(forceCurlDownload OFF)
if(MSVC)
    set(forceCurlDownload ON)
endif()

CPMAddPackage(
        NAME CURL
        GITHUB_REPOSITORY curl/curl
        GIT_TAG curl-8_7_1
        FORCE ${forceCurlDownload}
        OPTIONS "BUILD_CURL_EXE OFF"
        "BUILD_TESTING OFF"
        "CURL_USE_OPENSSL OFF"
        "CURL_USE_SCHANNEL ON"
)

find_package(OpenSSL REQUIRED)

# -----------------------------------------------------------------------
#  Targets & Helpers
# -----------------------------------------------------------------------
set(benchDataDir "${CMAKE_CURRENT_SOURCE_DIR}/data" CACHE PATH "Directory containing benchmark JSON data files")

function(ThothBenchTarget targetName)
    target_compile_definitions(${targetName} PRIVATE BENCH_DATA_DIR="${benchDataDir}")

    if(RapidJSON_ADDED)
        target_include_directories(${targetName} PRIVATE ${RapidJSON_SOURCE_DIR}/include)
    endif()

    target_link_libraries(${targetName} PRIVATE
            Thoth::Thoth
            benchmark::benchmark_main
            ${PROJECT_NAME}CompilerFlags
    )
endfunction()

# -----------------------------------------------------------------------
#  Sub-targets
# -----------------------------------------------------------------------
add_subdirectory(Json)
add_subdirectory(Http)
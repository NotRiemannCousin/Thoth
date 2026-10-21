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
        "HTTPLIB_USE_OPENSSL_IF_AVAILABLE ON"
        "HTTPLIB_INSTALL OFF"
)

# ── CURL (via CPM; uses Windows SChannel instead of OpenSSL) ─────────
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

find_package(Boost REQUIRED COMPONENTS json beast asio)

add_executable(ComparisonTests main.cpp Test0.cpp Test1.cpp Test2.cpp Test3.cpp Test4.cpp)

target_include_directories(ComparisonTests PRIVATE include)

target_link_libraries(
        ComparisonTests
        httplib::httplib
        OpenSSL::SSL
        OpenSSL::Crypto
        CURL::libcurl
        Thoth::Thoth
        nlohmann_json::nlohmann_json
        simdjson::simdjson
        rapidjson
        Boost::json
        Boost::asio
        Boost::beast
)
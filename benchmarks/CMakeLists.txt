include(FetchContent)

# -----------------------------------------------------------------------
#  Google Benchmark
# -----------------------------------------------------------------------
find_package(benchmark QUIET)
if(NOT benchmark_FOUND)
    FetchContent_Declare(
            googlebenchmark
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG        v1.9.1
            GIT_SHALLOW    TRUE
    )
    set(BENCHMARK_ENABLE_TESTING          OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS      OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL          OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_DOWNLOAD_DEPENDENCIES   ON  CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googlebenchmark)
endif()

# -----------------------------------------------------------------------
#  JSON competitors
# -----------------------------------------------------------------------

# nlohmann/json  (header-only, no tests)
find_package(nlohmann_json QUIET)
if(NOT nlohmann_json_FOUND)
    FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG        v3.11.3
            GIT_SHALLOW    TRUE
    )
    set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
    set(JSON_Install    OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(nlohmann_json)
endif()

# simdjson
find_package(simdjson QUIET)
if(NOT simdjson_FOUND)
    FetchContent_Declare(
            simdjson
            GIT_REPOSITORY https://github.com/simdjson/simdjson.git
            GIT_TAG        v3.11.4
            GIT_SHALLOW    TRUE
    )
    set(SIMDJSON_ENABLE_THREADS   OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(simdjson)
endif()

# -----------------------------------------------------------------------
# RapidJSON (header-only)
# -----------------------------------------------------------------------
# Procura pelo header principal no sistema em vez de usar o find_package falho do RapidJSON
find_path(RAPIDJSON_SYS_INCLUDE_DIR NAMES rapidjson/document.h)

if(RAPIDJSON_SYS_INCLUDE_DIR)
    # Encontrou localmente
    set(RAPIDJSON_LOCAL_INCLUDE_DIR ${RAPIDJSON_SYS_INCLUDE_DIR})
else()
    # Não encontrou, baixa via FetchContent
    FetchContent_Declare(
            rapidjson
            GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
            GIT_TAG        master
            GIT_SHALLOW    TRUE
    )
    set(RAPIDJSON_BUILD_DOC       OFF CACHE BOOL "" FORCE)
    set(RAPIDJSON_BUILD_EXAMPLES  OFF CACHE BOOL "" FORCE)
    set(RAPIDJSON_BUILD_TESTS     OFF CACHE BOOL "" FORCE)
    set(RAPIDJSON_BUILD_CRT_LINKAGE ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(rapidjson)

    # Usa a variável fonte do FetchContent
    set(RAPIDJSON_LOCAL_INCLUDE_DIR "${rapidjson_SOURCE_DIR}/include")
endif()

# -----------------------------------------------------------------------
#  HTTP competitors
# -----------------------------------------------------------------------

# cpp-httplib  (header-only)
find_package(httplib QUIET)
if(NOT httplib_FOUND)
    FetchContent_Declare(
            cpp_httplib
            GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
            GIT_TAG        v0.18.3
            GIT_SHALLOW    TRUE
    )
    set(HTTPLIB_COMPILE        ON  CACHE BOOL "" FORCE)
    set(HTTPLIB_USE_OPENSSL_IF_AVAILABLE OFF CACHE BOOL "" FORCE) # Desativado para evitar conflitos MSYS2
    set(HTTPLIB_INSTALL        OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(cpp_httplib)
endif()

# libcurl
# Evita o cURL do MSYS2 no MSVC. Baixa da fonte e usa a criptografia nativa do Windows (Schannel).
find_package(CURL QUIET)
if(NOT CURL_FOUND OR "${CURL_DIR}" MATCHES "msys64")
    FetchContent_Declare(
            curl
            GIT_REPOSITORY https://github.com/curl/curl.git
            GIT_TAG        curl-8_7_1
            GIT_SHALLOW    TRUE
    )
    set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING  OFF CACHE BOOL "" FORCE)
    set(CURL_USE_OPENSSL  OFF CACHE BOOL "" FORCE)
    set(CURL_USE_SCHANNEL ON  CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(curl)
endif()
find_package(OpenSSL REQUIRED)

# -----------------------------------------------------------------------
#  Helper: propagate the benchmark data path to executables
# -----------------------------------------------------------------------
set(BENCH_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data"
        CACHE PATH "Directory containing benchmark JSON data files")

# -----------------------------------------------------------------------
#  Shared compile helper
# -----------------------------------------------------------------------
function(thoth_bench_target TARGET)
    target_compile_features(${TARGET} PRIVATE cxx_std_23)

    target_include_directories(${TARGET} PRIVATE
            ${RAPIDJSON_LOCAL_INCLUDE_DIR}
    )

    target_compile_definitions(${TARGET} PRIVATE
            BENCH_DATA_DIR="${BENCH_DATA_DIR}"
    )

    # MSVC-specific hardening
    target_compile_options(${TARGET} PRIVATE
            /permissive-
            /W4
            /Zc:preprocessor
    )

    target_link_libraries(${TARGET} PRIVATE
            Thoth
            benchmark::benchmark
            benchmark::benchmark_main
    )
endfunction()

# -----------------------------------------------------------------------
#  Sub-targets
# -----------------------------------------------------------------------
add_subdirectory(Json)
add_subdirectory(Http)
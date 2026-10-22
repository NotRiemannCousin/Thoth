# ── yyjson (C library, via vcpkg) ─────────────────────────────
find_package(yyjson CONFIG REQUIRED)

# ── Boost.JSON (via vcpkg) ──────────────────────────────────────
# find_package(Boost ... COMPONENTS json) defines Boost::json (header-only
# usage here; the compiled boost_json-* lib is linked transitivamente).
find_package(Boost REQUIRED COMPONENTS json)

set(BENCH_JSON_SOURCES
    BenchJson_Thoth.cpp
    BenchJson_Nlohmann.cpp
    BenchJson_Simdjson.cpp
    BenchJson_Rapidjson.cpp
    BenchJson_Boost.cpp
    BenchJson_Yyjson.cpp
)

add_executable(BenchJson ${BENCH_JSON_SOURCES})

ThothBenchTarget(BenchJson)

CPMAddPackage(
        NAME mimalloc
        GITHUB_REPOSITORY microsoft/mimalloc
        GIT_TAG v3.5.3
)

if(MSVC)
    target_compile_options(BenchJson PRIVATE /Zc:preprocessor)
endif()

target_link_libraries(BenchJson PRIVATE
        nlohmann_json::nlohmann_json
        simdjson::simdjson
        rapidjson
        mimalloc-static
        Boost::json
        yyjson::yyjson
)

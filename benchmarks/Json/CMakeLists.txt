add_executable(BenchJson BenchJson.cpp)

ThothBenchTarget(BenchJson)

CPMAddPackage(
        NAME mimalloc
        GITHUB_REPOSITORY microsoft/mimalloc
        GIT_TAG v3.5.3
)

target_link_libraries(BenchJson PRIVATE
        nlohmann_json::nlohmann_json
        simdjson::simdjson
        rapidjson
        mimalloc-static
)
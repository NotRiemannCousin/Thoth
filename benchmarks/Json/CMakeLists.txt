add_executable(BenchJson BenchJson.cpp)

thoth_bench_target(BenchJson)

target_link_libraries(BenchJson PRIVATE
        nlohmann_json::nlohmann_json
        simdjson::simdjson
)
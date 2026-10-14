add_executable(BenchJson BenchJson.cpp)

ThothBenchTarget(BenchJson)

target_link_libraries(BenchJson PRIVATE
        nlohmann_json::nlohmann_json
        simdjson::simdjson
        rapidjson
)
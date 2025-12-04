#include <string>
#include <print>
#include <chrono>
#include <fstream>
#include <iostream>

// #define COMPARE_NLOHMANN

#include <Thoth/NJson/Json.hpp>

#ifdef COMPARE_NLOHMANN
#include "json.hpp" // https://github.com/nlohmann/json
#endif


template<auto Callable>
void TimeIt(std::string_view name) {
    const auto start{ std::chrono::steady_clock::now() };
    Callable();
    const auto end{ std::chrono::steady_clock::now() };

    std::println("{:10}: {:.5f}s", name, std::chrono::duration<double>(end - start).count());
}

std::string data;

// decltype(nlohmann::json::parse(data)) nlohmannJson;
decltype(Thoth::NJson::Json::Parse(data)) thothJson;

#ifdef COMPARE_NLOHMANN
void NlohmannCreate(){
    // nlohmann::ordered_map<std::string, int> map;
    // for (std::string name : names)
    //     map.push_back({ std::move(name), 0 });
    nlohmannJson = nlohmann::json::parse(data);
}
#endif

void ThothCreate(){
    // std::map<std::string, int> map;
    // for (std::string name : names)
    //     map.insert({std::move(name), 0});
    thothJson = Thoth::NJson::Json::Parse(data);
}

#ifdef COMPARE_NLOHMANN
void NlohmannToString(){
    auto str = nlohmannJson.dump();
    // std::println(std::clog, "{}", nlohmannJson.dump());
}
#endif

void ThothToString(){
    auto str = std::format("{}", *thothJson);
    // std::println(std::clog, "{}", *thothJson);
}


int main() {
    auto file = std::ifstream("data", std::ios::binary);

    data.reserve(1e6);
    std::string line;
    if (!file.is_open()) {
        // ReSharper disable once CppDeprecatedEntity
        std::println("Erro: {}", strerror(errno));
    }

    while (file.is_open() && !file.eof()) {
        std::getline(file, line);

        data.append_range(line);
    }

    std::println("\n\tCreate\n");
#ifdef COMPARE_NLOHMANN
    TimeIt<NlohmannCreate>("nlohmann");
#endif
    TimeIt<ThothCreate>("Thoth");


    std::println("\n\tDump\n");
#ifdef COMPARE_NLOHMANN
    TimeIt<NlohmannToString>("nlohmann");
#endif
    TimeIt<ThothToString>("Thoth");

    return 0;
}


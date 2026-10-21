#pragma once
#include <optional>
#include <vector>
#include <string>

namespace Test1 {
    // httplib + RapidJSON

    std::optional<std::vector<std::string>> GetMembers(size_t id);
}


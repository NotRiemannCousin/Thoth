#pragma once
#include <optional>
#include <vector>
#include <string>

namespace Test0 {
    // httplib + nlohmann

    std::optional<std::vector<std::string>> GetMembers(size_t id);
}

#pragma once
#include <optional>
#include <vector>
#include <string>

namespace Test3 {
    // curllib + simdJSON

    std::optional<std::vector<std::string>> GetMembers(size_t id);
}
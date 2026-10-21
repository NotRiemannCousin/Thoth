#pragma once
#include <Thoth/Http/Client/Client.hpp>
#include <Thoth/NJson/Json.hpp>

namespace Test4 {
    // Thoth

    std::expected<std::vector<std::string>, Thoth::ThothError> GetMembers(size_t id);
}
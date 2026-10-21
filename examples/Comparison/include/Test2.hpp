#pragma once
#include <boost/system/error_code.hpp>
#include <expected>
#include <vector>

namespace Test2 {
    // BoostBeast + BoostJSON

    std::expected<std::vector<std::string>, boost::system::error_code> GetMembers(size_t id);
}
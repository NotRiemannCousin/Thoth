#pragma once

#include <ranges>
#include <utility>

namespace Thoth::Utils {

    constexpr auto FlatMap(auto fn) {
        return std::views::transform(std::move(fn)) | std::views::join;
    }
}

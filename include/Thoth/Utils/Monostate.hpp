#pragma once

#include <format>
#include <variant>

template<>
struct std::formatter<std::monostate> {
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    auto format(const std::monostate, auto& ctx) const {
        return std::format_to(ctx.out(), "null");
    }
};

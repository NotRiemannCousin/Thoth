#pragma once
#include <format>
;

template<>
struct std::formatter<std::monostate> {
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    auto format(const std::monostate, auto& ctx) const {
        std::format_to(ctx.out(), "null");

        return ctx.out();
    }
};
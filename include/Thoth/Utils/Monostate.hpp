#pragma once
#include <format>
;

template<>
struct std::formatter<std::monostate> {
    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const std::monostate, FormatContext& ctx) const {
        std::format_to(ctx.out(), "null");

        return ctx.out();
    }
};
#pragma once
#include <format>

template<>
struct std::formatter<Thoth::Http::ResponseHead> {
    constexpr auto parse(auto& ctx) { return ctx.begin(); }

    auto format(const Thoth::Http::ResponseHead& head, auto& ctx) const {
        return std::format_to(ctx.out(), "{} {}\r\n{}\r\n", head.version, head.status, head.headers);
    }
};


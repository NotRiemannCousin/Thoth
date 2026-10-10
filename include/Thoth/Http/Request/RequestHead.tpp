#pragma once
#include <format>

template<>
struct std::formatter<Thoth::Http::RequestHead> {
    constexpr auto parse(auto& ctx) { return ctx.begin(); }

    auto format(const Thoth::Http::RequestHead& head, auto& ctx) const {
        using Thoth::Http::details_::k_crlf;
        const auto query{ head.url.GetQuery() };

        auto it{ std::format_to(ctx.out(), "{}", head.url.GetPathOrSep()) };
        if (!query.empty()) it = std::format_to(it, "?{}", query);

        return std::format_to(it, " {}\r\n{}\r\n", head.version, head.headers);
    }
};
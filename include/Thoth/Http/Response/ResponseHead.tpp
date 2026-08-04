#pragma once
#include <format>

template<>
struct std::formatter<Thoth::Http::ResponseHead> {
    template<class FormatContext>
    constexpr auto parse(FormatContext& ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::ResponseHead& head, FormatContext& ctx) const {
        using Thoth::Http::details_::k_crlf;

        return std::format_to(ctx.out(), "{} {} {}\r\n{}\r\n", head.version,
                std::to_underlying(head.status), head.statusMessage, head.headers);
    }
};


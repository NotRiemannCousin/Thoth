#pragma once
#include <ranges>

namespace std {
    template<>
    struct formatter<Thoth::Http::HttpHeaders>{

        static constexpr auto parse(auto &ctx) { return ctx.begin(); }

        auto format(const Thoth::Http::HttpHeaders &headers, std::format_context  &ctx) const {
            for (const auto& p : headers._headers)
                format_to(ctx.out(), "{}: {}\r\n", p.first, p.second);

            return ctx.out();
        }
    };
}

inline auto Thoth::Http::HttpHeaders::GetSetCookieView() const {
    constexpr auto cmp = [](const auto& p) {
        return p.first == "set-cookie";
    };


    return _headers
            | std::views::filter(cmp)
            | std::views::transform(&HeaderPair::second);
    // TODO: move to cpp?
}

#pragma once

namespace std {
    template<>
    struct hash<Thoth::Http::Url> {
        size_t operator()(const Thoth::Http::Url& url) const noexcept {
            return hash<std::string>()(url.rawUrl);
        }
    };

    template<>
    struct formatter<Thoth::Http::Url>{
        bool origin{};

        constexpr auto parse(auto &ctx) {
            auto it{ ctx.begin() };
            if (it != ctx.end() && (*it == 'o' || *it == 'O'))
                origin = true, ++it;
            return it;
        }

        template<class FormatContext>
        auto format(const Thoth::Http::Url &url, FormatContext& ctx) const {
            format_to(ctx.out(), "{}", url.rawUrl);

            return ctx.out();
        }
    };
}

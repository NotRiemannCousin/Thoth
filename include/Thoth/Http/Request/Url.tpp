#pragma once

namespace std {
    template<>
    struct hash<Thoth::Http::Url> {
        size_t operator()(const Thoth::Http::Url& url) const noexcept {
            return
                (hash<string>()(url.scheme)                  << 0) ^
                (hash<string>()(url.user)                    << 1) ^
                (hash<string>()(url.host)                    << 2) ^
                (hash<int>()(url.port)                       << 3) ^
                (hash<string>()(url.path)                    << 4) ^
                (hash<Thoth::Http::QueryParams>()(url.query) << 5) ^
                (hash<string>()(url.fragment)                << 6);
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

        auto format(const Thoth::Http::Url &url, std::format_context &ctx) const {
            format_to(ctx.out(), "{}://", url.scheme);

            if (!origin && !url.user.empty())
                format_to(ctx.out(), "{}@", url.user);


            format_to(ctx.out(), "{}", url.host);

            if (url.port != 0)
                format_to(ctx.out(), ":{}", url.port);


            if (origin)
                return ctx.out();


            format_to(ctx.out(), "{}", url.path);


            if (!url.query.Empty())
                format_to(ctx.out(), "?{}", url.query);


            if (!url.fragment.empty())
                format_to(ctx.out(), "#{}", url.fragment);

            return ctx.out();
        }
    };
}

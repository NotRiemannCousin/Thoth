#pragma once

namespace std {
    // TODO: hash

    template<>
    struct formatter<Thoth::Http::HttpUrl>{
        bool origin{};

        constexpr auto parse(auto &ctx) {
            auto it{ ctx.begin() };
            if (it != ctx.end() && (*it == 'o' || *it == 'O'))
                origin = true, it++;
            return it;
        }

        auto format(const Thoth::Http::HttpUrl &url, std::format_context &ctx) const {
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

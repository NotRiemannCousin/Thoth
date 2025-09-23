#pragma once

namespace std {
    // TODO: hash

    template<>
    struct formatter<Thoth::Http::HttpUrl>{
        static constexpr auto parse(auto &ctx) { return ctx.begin(); }

        auto format(const Thoth::Http::HttpUrl &url, std::format_context &ctx) const {
            format_to(ctx.out(), "{}://", url.scheme);

            if (!url.user.empty())
                format_to(ctx.out(), "{}@", url.user);


            format_to(ctx.out(), "{}", url.host);

            if (url.port != 0)
                format_to(ctx.out(), ":{}", url.port);



            format_to(ctx.out(), "{}", url.path);


            if (!url.query.Empty())
                format_to(ctx.out(), "?{}", url.query);


            if (!url.fragment.empty())
                format_to(ctx.out(), "#{}", url.fragment);

            return ctx.out();
        }
    };
}

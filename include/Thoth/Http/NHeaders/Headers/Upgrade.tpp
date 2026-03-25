#pragma once

template<>
struct Thoth::Http::NHeaders::Scanner<Thoth::Http::NHeaders::Upgrade> {
    static bool Parse(const std::string_view str) {
        return str.empty();
    }

    std::optional<Upgrade> Scan(string_view input) {
        String::Trim(input);
        const auto sep{ input.find('/') };
        Upgrade upgrade{ string{ input.substr(0, sep) } };

        if (sep == string::npos || input.substr(sep + 1).empty())
            upgrade.version = input;



        return upgrade;
    }
};

template<>
struct std::formatter<Thoth::Http::NHeaders::Upgrade> {
    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::NHeaders::Upgrade& upgrade, FormatContext& ctx) const {
        using Thoth::Http::NHeaders::Upgrade;

        std::format_to(ctx.out(), "{}", upgrade.protocol);
        if (upgrade.version) std::format_to(ctx.out(), "/{}", *upgrade.version);

        return ctx.out();
    }
};

namespace Thoth::Http::NHeaders {
    static_assert(Serializable<Upgrade>);
}
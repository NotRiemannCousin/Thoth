#pragma once

template<>
struct Thoth::Utils::Scanner<Thoth::Http::NHeaders::Upgrade> {
    using Upgrade = Http::NHeaders::Upgrade;

    static bool Parse(const std::string_view str) {
        return str.empty();
    }

    std::optional<Upgrade> Scan(std::string_view input) {
        String::Trim(input);
        const auto sep{ input.find('/') };
        Upgrade upgrade{ std::string{ input.substr(0, sep) } };

        if (sep != std::string::npos && !input.substr(sep + 1).empty())
            upgrade.version = input.substr(sep + 1);


        return upgrade;
    }
};

template<>
struct std::formatter<Thoth::Http::NHeaders::Upgrade> {
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    auto format(const Thoth::Http::NHeaders::Upgrade& upgrade, auto& ctx) const {
        using Thoth::Http::NHeaders::Upgrade;
        auto out{ ctx.out() };

        out = std::format_to(out, "{}", upgrade.protocol);
        if (upgrade.version) out = std::format_to(out, "/{}", *upgrade.version);

        return out;
    }
};

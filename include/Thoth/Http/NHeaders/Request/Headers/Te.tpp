#pragma once
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>
#include <algorithm>
#include <optional>
#include <ranges>
#include <format>

#include <Thoth/String/Utils.hpp>



template<>
struct Thoth::Utils::Scanner<Thoth::Http::NHeaders::TeEnum> {
    using TeEnum = Http::NHeaders::TeEnum;

    static bool Parse(const std::string_view str) {
        return str.empty();
    }

    std::optional<TeEnum> Scan(std::string_view input) {
        String::Trim(input);
        const auto value{ input | std::views::transform(tolower) };

        if (std::ranges::equal(value, std::string_view{ "compress" })) return TeEnum::Compress;
        if (std::ranges::equal(value, std::string_view{ "deflate"  })) return TeEnum::Deflate;
        if (std::ranges::equal(value, std::string_view{ "gzip"     })) return TeEnum::Gzip;
        if (std::ranges::equal(value, std::string_view{ "trailers" })) return TeEnum::Trailers;

        return std::nullopt;
    }
};

template<>
struct std::formatter<Thoth::Http::NHeaders::TeEnum> {
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    auto format(const Thoth::Http::NHeaders::TeEnum te, auto& ctx) const {
        using Thoth::Http::NHeaders::TeEnum;

        switch (te) {
            case TeEnum::Compress: return std::format_to(ctx.out(), "compress");
            case TeEnum::Deflate : return std::format_to(ctx.out(), "deflate" );
            case TeEnum::Gzip    : return std::format_to(ctx.out(), "gzip"    );
            case TeEnum::Trailers: return std::format_to(ctx.out(), "trailers");
        }

        return ctx.out();
    }
};

#pragma once
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>
#include <algorithm>
#include <optional>
#include <ranges>
#include <format>

#include <Thoth/String/Utils.hpp>



template<>
struct Thoth::Http::NHeaders::Scanner<Thoth::Http::NHeaders::TeEnum> {
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
    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::NHeaders::TeEnum te, FormatContext& ctx) const {
        using Thoth::Http::NHeaders::TeEnum;

        if (te == TeEnum::Compress) std::format_to(ctx.out(), "compress");
        if (te == TeEnum::Deflate ) std::format_to(ctx.out(), "deflate" );
        if (te == TeEnum::Gzip    ) std::format_to(ctx.out(), "gzip"    );
        if (te == TeEnum::Trailers) std::format_to(ctx.out(), "trailers");

        return ctx.out();
    }
};

namespace Thoth::Http::NHeaders {
    static_assert(Serializable<TeEnum>);
}
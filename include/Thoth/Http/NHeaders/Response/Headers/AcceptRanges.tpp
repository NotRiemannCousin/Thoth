#pragma once
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>
#include <algorithm>
#include <optional>
#include <ranges>
#include <format>

#include <Thoth/String/Utils.hpp>



template<>
struct Thoth::Utils::Scanner<Thoth::Http::NHeaders::AcceptRanges> {
    using AcceptRanges = Http::NHeaders::AcceptRanges;

    static bool Parse(const std::string_view str) {
        return str.empty();
    }

    std::optional<AcceptRanges> Scan(std::string_view input) {
        String::Trim(input);
        const auto value{ input | std::views::transform(tolower) };

        if (std::ranges::equal(value, std::string_view{ "none"  })) return AcceptRanges::None;
        if (std::ranges::equal(value, std::string_view{ "bytes" })) return AcceptRanges::Bytes;

        return std::nullopt;
    }
};

template<>
struct std::formatter<Thoth::Http::NHeaders::AcceptRanges> {
    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::NHeaders::AcceptRanges acceptRanges, FormatContext& ctx) const {
        using Thoth::Http::NHeaders::AcceptRanges;

        if (acceptRanges == AcceptRanges::None ) std::format_to(ctx.out(), "none" );
        if (acceptRanges == AcceptRanges::Bytes) std::format_to(ctx.out(), "bytes");

        return ctx.out();
    }
};

static_assert(Thoth::Utils::Serializable<Thoth::Http::NHeaders::AcceptRanges>);
#pragma once
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>
#include <format>

template<>
struct Thoth::Http::NHeaders::Scanner<Thoth::Http::NHeaders::TransferEncodingEnum> {
    static bool Parse(const std::string_view str) {
        return str.empty();
    }

    std::optional<TransferEncodingEnum> Scan(std::string_view input) {
        String::Trim(input);
        const auto value{ input | std::views::transform(tolower) };

        if (std::ranges::equal(value, std::string_view{ "chunked"  })) return TransferEncodingEnum::Chunked;
        if (std::ranges::equal(value, std::string_view{ "compress" })) return TransferEncodingEnum::Compress;
        if (std::ranges::equal(value, std::string_view{ "deflate"  })) return TransferEncodingEnum::Deflate;
        if (std::ranges::equal(value, std::string_view{ "gzip"     })) return TransferEncodingEnum::Gzip;

        return std::nullopt;
    }
};

template<>
struct std::formatter<Thoth::Http::NHeaders::TransferEncodingEnum> {
    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::NHeaders::TransferEncodingEnum transfer, FormatContext& ctx) const {
        using Thoth::Http::NHeaders::TransferEncodingEnum;

        if (transfer == TransferEncodingEnum::Chunked ) std::format_to(ctx.out(), "chunked" );
        if (transfer == TransferEncodingEnum::Compress) std::format_to(ctx.out(), "compress");
        if (transfer == TransferEncodingEnum::Gzip    ) std::format_to(ctx.out(), "gzip"    );
        if (transfer == TransferEncodingEnum::Deflate ) std::format_to(ctx.out(), "deflate" );

        return ctx.out();
    }
};
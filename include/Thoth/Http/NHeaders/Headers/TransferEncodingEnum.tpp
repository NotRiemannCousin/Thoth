#pragma once
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>
#include <format>

namespace Thoth::Http::NHeaders{
    template<>
    inline std::optional<TransferEncodingEnum> Scan<TransferEncodingEnum>(string_view input) {
        String::Trim(input);
        const auto value{ input | std::views::transform(tolower) };

        if (std::ranges::equal(value, string_view{ "chunked"  })) return TransferEncodingEnum::Chunked;
        if (std::ranges::equal(value, string_view{ "compress" })) return TransferEncodingEnum::Compress;
        if (std::ranges::equal(value, string_view{ "deflate"  })) return TransferEncodingEnum::Deflate;
        if (std::ranges::equal(value, string_view{ "gzip"     })) return TransferEncodingEnum::Gzip;

        return std::nullopt;
    }
}

template<>
struct std::formatter<Thoth::Http::NHeaders::TransferEncodingEnum> {
    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    static auto format(const Thoth::Http::NHeaders::TransferEncodingEnum transfer, std::format_context &ctx) {
        using Thoth::Http::NHeaders::TransferEncodingEnum;

        if (transfer == TransferEncodingEnum::Chunked ) std::format_to(ctx.out(), "chunked" );
        if (transfer == TransferEncodingEnum::Compress) std::format_to(ctx.out(), "compress");
        if (transfer == TransferEncodingEnum::Gzip    ) std::format_to(ctx.out(), "gzip"    );
        if (transfer == TransferEncodingEnum::Deflate ) std::format_to(ctx.out(), "deflate" );

        return ctx.out();
    }
};
#pragma once
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>
#include <format>

template<>
struct Thoth::Utils::Scanner<Thoth::Http::NHeaders::TransferEncodingEnum> {
    using TransferEncodingEnum = Http::NHeaders::TransferEncodingEnum;

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
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    auto format(const Thoth::Http::NHeaders::TransferEncodingEnum transfer, auto& ctx) const {
        using Thoth::Http::NHeaders::TransferEncodingEnum;

        switch (transfer) {
            case TransferEncodingEnum::Chunked : return std::format_to(ctx.out(), "chunked" );
            case TransferEncodingEnum::Compress: return std::format_to(ctx.out(), "compress");
            case TransferEncodingEnum::Gzip    : return std::format_to(ctx.out(), "gzip"    );
            case TransferEncodingEnum::Deflate : return std::format_to(ctx.out(), "deflate" );
        }

        return ctx.out();
    }
};

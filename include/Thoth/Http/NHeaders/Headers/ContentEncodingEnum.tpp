#pragma once
#include <string_view>
#include <ranges>

#include <Thoth/Http/NHeaders/Proxy/_base.hpp>

template<>
struct Thoth::Utils::Scanner<Thoth::Http::NHeaders::ContentEncodingEnum> {
    using ContentEncodingEnum = Http::NHeaders::ContentEncodingEnum;

    static bool Parse(const std::string_view str) {
        return str.empty();
    }

    std::optional<ContentEncodingEnum> Scan(std::string_view input) {
        String::Trim(input);
        const auto value{ input | std::views::transform(tolower) };

        if (std::ranges::equal(value, std::string_view{ "gzip"     })) return ContentEncodingEnum::Gzip;
        if (std::ranges::equal(value, std::string_view{ "compress" })) return ContentEncodingEnum::Compress;
        if (std::ranges::equal(value, std::string_view{ "deflate"  })) return ContentEncodingEnum::Deflate;
        if (std::ranges::equal(value, std::string_view{ "br"       })) return ContentEncodingEnum::Br;
        if (std::ranges::equal(value, std::string_view{ "zstd"     })) return ContentEncodingEnum::Zstd;
        if (std::ranges::equal(value, std::string_view{ "dcb"      })) return ContentEncodingEnum::Dcb;
        if (std::ranges::equal(value, std::string_view{ "dcz"      })) return ContentEncodingEnum::Dcz;

        return std::nullopt;
    }
};

template<>
struct std::formatter<Thoth::Http::NHeaders::ContentEncodingEnum> {
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    auto format(const Thoth::Http::NHeaders::ContentEncodingEnum contentEncoding, auto& ctx) const {
        using Thoth::Http::NHeaders::ContentEncodingEnum;

        switch (contentEncoding) {
            case ContentEncodingEnum::Gzip    : return std::format_to(ctx.out(), "gzip"    );
            case ContentEncodingEnum::Compress: return std::format_to(ctx.out(), "compress");
            case ContentEncodingEnum::Deflate : return std::format_to(ctx.out(), "deflate" );
            case ContentEncodingEnum::Br      : return std::format_to(ctx.out(), "br"      );
            case ContentEncodingEnum::Zstd    : return std::format_to(ctx.out(), "zstd"    );
            case ContentEncodingEnum::Dcb     : return std::format_to(ctx.out(), "dcb"     );
            case ContentEncodingEnum::Dcz     : return std::format_to(ctx.out(), "dcz"     );
        }

        return ctx.out();
    }
};

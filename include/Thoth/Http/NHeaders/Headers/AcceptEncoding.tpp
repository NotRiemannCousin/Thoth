#pragma once
#include <string_view>
#include <ranges>

#include <Thoth/Http/NHeaders/Proxy/_base.hpp>

template<>
struct Thoth::Utils::Scanner<Thoth::Http::NHeaders::AcceptEncodingEnum> {
    using AcceptEncodingEnum = Http::NHeaders::AcceptEncodingEnum;

    static bool Parse(const std::string_view str) {
        return str.empty();
    }

    std::optional<AcceptEncodingEnum> Scan(std::string_view input) {
        String::Trim(input);
        const auto value{ input | std::views::transform(tolower) };

        if (std::ranges::equal(value, std::string_view{ "gzip"     })) return AcceptEncodingEnum::Gzip;
        if (std::ranges::equal(value, std::string_view{ "compress" })) return AcceptEncodingEnum::Compress;
        if (std::ranges::equal(value, std::string_view{ "deflate"  })) return AcceptEncodingEnum::Deflate;
        if (std::ranges::equal(value, std::string_view{ "br"       })) return AcceptEncodingEnum::Br;
        if (std::ranges::equal(value, std::string_view{ "zstd"     })) return AcceptEncodingEnum::Zstd;
        if (std::ranges::equal(value, std::string_view{ "dcb"      })) return AcceptEncodingEnum::Dcb;
        if (std::ranges::equal(value, std::string_view{ "dcz"      })) return AcceptEncodingEnum::Dcz;
        if (std::ranges::equal(value, std::string_view{ "*"        })) return AcceptEncodingEnum::Identity;

        return std::nullopt;
    }
};

template<>
struct std::formatter<Thoth::Http::NHeaders::AcceptEncodingEnum> {
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    auto format(const Thoth::Http::NHeaders::AcceptEncodingEnum acceptEncoding, auto& ctx) const {
        using Thoth::Http::NHeaders::AcceptEncodingEnum;

        switch (acceptEncoding) {
            case AcceptEncodingEnum::Gzip    : return std::format_to(ctx.out(), "gzip"    );
            case AcceptEncodingEnum::Compress: return std::format_to(ctx.out(), "compress");
            case AcceptEncodingEnum::Deflate : return std::format_to(ctx.out(), "deflate" );
            case AcceptEncodingEnum::Br      : return std::format_to(ctx.out(), "br"      );
            case AcceptEncodingEnum::Zstd    : return std::format_to(ctx.out(), "zstd"    );
            case AcceptEncodingEnum::Dcb     : return std::format_to(ctx.out(), "dcb"     );
            case AcceptEncodingEnum::Dcz     : return std::format_to(ctx.out(), "dcz"     );
            case AcceptEncodingEnum::Identity: return std::format_to(ctx.out(), "identity");
            case AcceptEncodingEnum::Wildcard: return std::format_to(ctx.out(), "*"       );
        }

        return ctx.out();
    }
};

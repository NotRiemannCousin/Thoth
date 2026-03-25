#pragma once
#include <string_view>
#include <ranges>

#include <Thoth/Http/NHeaders/Proxy/_base.hpp>

template<>
struct Thoth::Http::NHeaders::Scanner<Thoth::Http::NHeaders::AcceptEncodingEnum> {
    static bool Parse(const std::string_view str) {
        return str.empty();
    }

    std::optional<AcceptEncodingEnum> Scan(string_view input) {
        String::Trim(input);
        const auto value{ input | std::views::transform(tolower) };

        if (std::ranges::equal(value, string_view{ "gzip"     })) return AcceptEncodingEnum::Gzip;
        if (std::ranges::equal(value, string_view{ "compress" })) return AcceptEncodingEnum::Compress;
        if (std::ranges::equal(value, string_view{ "deflate"  })) return AcceptEncodingEnum::Deflate;
        if (std::ranges::equal(value, string_view{ "br"       })) return AcceptEncodingEnum::Br;
        if (std::ranges::equal(value, string_view{ "zstd"     })) return AcceptEncodingEnum::Zstd;
        if (std::ranges::equal(value, string_view{ "dcb"      })) return AcceptEncodingEnum::Dcb;
        if (std::ranges::equal(value, string_view{ "dcz"      })) return AcceptEncodingEnum::Dcz;
        if (std::ranges::equal(value, string_view{ "*"        })) return AcceptEncodingEnum::Identity;

        return std::nullopt;
    }
};

template<>
struct std::formatter<Thoth::Http::NHeaders::AcceptEncodingEnum> {
    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::NHeaders::AcceptEncodingEnum acceptEncoding, FormatContext& ctx) const {
        using Thoth::Http::NHeaders::AcceptEncodingEnum;

        if (acceptEncoding == AcceptEncodingEnum::Gzip    ) std::format_to(ctx.out(), "gzip"    );
        if (acceptEncoding == AcceptEncodingEnum::Compress) std::format_to(ctx.out(), "compress");
        if (acceptEncoding == AcceptEncodingEnum::Deflate ) std::format_to(ctx.out(), "deflate" );
        if (acceptEncoding == AcceptEncodingEnum::Br      ) std::format_to(ctx.out(), "br"      );
        if (acceptEncoding == AcceptEncodingEnum::Zstd    ) std::format_to(ctx.out(), "zstd"    );
        if (acceptEncoding == AcceptEncodingEnum::Dcb     ) std::format_to(ctx.out(), "dcb"     );
        if (acceptEncoding == AcceptEncodingEnum::Dcz     ) std::format_to(ctx.out(), "dcz"     );
        if (acceptEncoding == AcceptEncodingEnum::Identity) std::format_to(ctx.out(), "identity");
        if (acceptEncoding == AcceptEncodingEnum::Wildcard) std::format_to(ctx.out(), "*"       );

        return ctx.out();
    }
};

namespace Thoth::Http::NHeaders {
    static_assert(Serializable<AcceptEncodingEnum>);
}

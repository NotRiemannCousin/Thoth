#pragma once
#include <format>

template<>
struct std::formatter<Thoth::Http::VersionEnum> {
    template<class FormatContext>
    constexpr auto parse(FormatContext& ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::VersionEnum version, FormatContext& ctx) const {
        using Thoth::Http::VersionEnum;

        switch (version) {
            case VersionEnum::HTTP1_0: return std::format_to(ctx.out(), "HTTP/1.0");
            case VersionEnum::HTTP1_1: return std::format_to(ctx.out(), "HTTP/1.1");
            case VersionEnum::HTTP2:   return std::format_to(ctx.out(), "HTTP/2");
            case VersionEnum::HTTP3:   return std::format_to(ctx.out(), "HTTP/3");
            default:                   return std::format_to(ctx.out(), "HTTP/1.1");
        }
    }
};
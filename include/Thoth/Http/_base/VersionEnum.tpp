#pragma once
#include <format>

constexpr std::string_view Thoth::Http::VersionToString(const VersionEnum version) {
    switch (version) {
        case VersionEnum::Http1_0: return "HTTP/1.0";
        case VersionEnum::Http1_1: return "HTTP/1.1";
        case VersionEnum::Http2:   return "HTTP/2";
        case VersionEnum::Http3:   return "HTTP/3";
        default: return "HTTP/1.1";
    }
}


template<>
struct std::formatter<Thoth::Http::VersionEnum> {
    constexpr auto parse(auto& ctx) { return ctx.begin(); }

    auto format(const Thoth::Http::VersionEnum version, auto& ctx) const {
        using Thoth::Http::VersionEnum;

        switch (version) {
            case VersionEnum::Http1_0: return std::format_to(ctx.out(), "HTTP/1.0");
            case VersionEnum::Http1_1: return std::format_to(ctx.out(), "HTTP/1.1");
            case VersionEnum::Http2:   return std::format_to(ctx.out(), "HTTP/2");
            case VersionEnum::Http3:   return std::format_to(ctx.out(), "HTTP/3");
            default:                   return std::format_to(ctx.out(), "HTTP/1.1");
        }
    }
};

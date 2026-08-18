#pragma once
#include <format>

template<>
struct std::formatter<Thoth::Http::UrlParseErrorEnum> {
    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::UrlParseErrorEnum err, FormatContext& ctx) const {
        constexpr const char* desc[]{ // will be changed to reflection in the future
            "EmptyUrl: The URL is empty",
            "InvalidScheme: Thoth only supports http or https",
            "IllFormed: unknown error, probably an invalid character",
            "HostIsRequired: no host found in URL",
            "InvalidPort: make sure that the port is a integer between 0 a 65535"
        };
        return std::format_to(ctx.out(), "{}", desc[to_underlying(err)]);
    }
};

template<>
struct std::formatter<Thoth::Http::MessageParseErrorEnum> {
    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::MessageParseErrorEnum err, FormatContext& ctx) const {
        constexpr const char* desc[]{
            "InvalidStartLine: unknown error, probably a invalid character",
            "InvalidVersion: uses 1.0 or 1.1 (2.0 and 3.0 in the future)",
            "InvalidHeaders: error while parsing headers, maybe invalid values for defined headers or invalid chars",
            "VersionNeedsContentLength: HTTP 1.0 needs the use of content-length"
        };

        return std::format_to(ctx.out(), "{}", desc[to_underlying(err)]);
    }
};

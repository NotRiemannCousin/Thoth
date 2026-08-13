#pragma once
#include <ranges>

#include <Hermes/Utils/Overloads.hpp>

template<>
struct std::formatter<Thoth::Http::ExchangeError> {
    using ExchangeError = Thoth::Http::ExchangeError;

    using JsonParseError        = Thoth::Http::JsonParseError;
    using JsonGetError          = Thoth::Http::JsonGetError;
    using JsonFindError         = Thoth::Http::JsonFindError;
    using JsonSearchError       = Thoth::Http::JsonSearchError;
    using UrlParseErrorEnum     = Thoth::Http::UrlParseErrorEnum;
    using ConnectionErrorEnum   = Thoth::Http::ConnectionErrorEnum;
    using MessageParseErrorEnum = Thoth::Http::MessageParseErrorEnum;
    using GenericError          = Thoth::Http::GenericError;

    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    template<class FormatContext>
    auto format(const ExchangeError &error, FormatContext& ctx) const {
        namespace rg = std::ranges;
        namespace vs = std::views;

        constexpr auto keyToStr{ [](const Thoth::NJson::Key& key) {
            return std::visit(
                Hermes::Utils::Overloaded{
                    [](std::string objKey) { return objKey; },
                    [](const int arrKey)   { return std::to_string(arrKey); }
                }, key
            );
        } };

        std::visit(
            Hermes::Utils::Overloaded{
                [&](const JsonParseError& e) {
                    std::format_to(ctx.out(), "Unknown character '{}' at position {}", e.c, e.idx);
                },
                [&](const JsonGetError& e) {
                    std::format_to(ctx.out(), "Can't find object with the '{}' key", keyToStr(e.key));
                },
                [&](const JsonFindError& e) {
                    std::string sla{ e.currentPath
                            | vs::transform(keyToStr)
                            | vs::join_with(string_view{ ", " })
                            | rg::to<string>() };
                    std::format_to(ctx.out(), "Unable to find '{}' in the tree [{}]", keyToStr(e.key), sla);
                },
                [&](const JsonSearchError& e) {
                    std::format_to(ctx.out(), "No object matches the predicate");
                },
                [&](const Thoth::Http::JsonWrongTypeError e) {
                    constexpr const char* types[] {
                        "null",
                        "string",
                        "number",
                        "bool",
                        "object",
                        "array"
                    };
                    std::format_to(ctx.out(), "Json has the wrong type, expecting '{}' but got '{}'", types[e.idxExpected], types[e.idxGot]);
                }
                ,
                [&](const UrlParseErrorEnum e) {
                    constexpr const char* desc[]{ // will be changed to reflection in the future
                        "EmptyUrl: The URL is empty",
                        "InvalidScheme: Thoth only supports http or https",
                        "IllFormed: unknown error, probably an invalid character",
                        "HostIsRequired: no host found in URL",
                        "InvalidPort: make sure that the port is a integer between 0 a 65535"
                    };
                    std::format_to(ctx.out(), "{}", desc[to_underlying(e)]);
                },
                [&](const ConnectionErrorEnum e) {
                    // constexpr const char* desc[]{
                    //     "EmptyUrl: The URL is empty",
                    //     "InvalidScheme: Thoth only supports http or https",
                    //     "IllFormed: unknown error, probably invalid character",
                    //     "InvalidPort: make sure that the port is a integer between 0 a 65535"
                    // };
                    // std::format_to(ctx.out(), "{}", desc[to_underlying(e)]);
                    std::format_to(ctx.out(), "{:v}", e);
                },
                [&](const MessageParseErrorEnum e) {
                    constexpr const char* desc[]{
                        "InvalidStartLine: unknown error, probably a invalid character",
                        "InvalidVersion: uses 1.0 or 1.1 (2.0 and 3.0 in the future)",
                        "InvalidHeaders: error while parsing headers, maybe invalid values for defined headers or invalid chars",
                        "VersionNeedsContentLength: HTTP 1.0 needs the use of content-length"
                    };
                    std::format_to(ctx.out(), "{}", desc[to_underlying(e)]);
                },
                [&](const GenericError& e) {
                    std::format_to(ctx.out(), "{}", e.error);
                }
            }, error
        );

        return ctx.out();
    }
};
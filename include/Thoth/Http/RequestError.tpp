#pragma once
#include <ranges>

#include <Thoth/Utils/Overloads.hpp>

template<>
struct std::formatter<Thoth::Http::RequestError> {
    using RequestError = Thoth::Http::RequestError;

    using JsonParseError = Thoth::Http::JsonParseError;
    using JsonGetError = Thoth::Http::JsonGetError;
    using JsonFindError = Thoth::Http::JsonFindError;
    using JsonSearchError = Thoth::Http::JsonSearchError;
    using UrlParseErrorEnum = Thoth::Http::UrlParseErrorEnum;
    using ConnectionErrorEnum = Thoth::Http::ConnectionErrorEnum;
    using RequestBuildErrorEnum = Thoth::Http::RequestBuildErrorEnum;
    using GenericError = Thoth::Http::GenericError;

    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const RequestError &error, std::format_context &ctx) const {
        constexpr auto s_keyToStr = [](const Thoth::NJson::Key& key) {
            return std::visit(
                Thoth::Utils::Overloaded{
                    [](string objKey) { return objKey; },
                    [](int arrKey) { return to_string(arrKey); }
                }, key
            );
        };

        std::visit(
            Thoth::Utils::Overloaded{
                [&](const JsonParseError& e) {
                    std::format_to(ctx.out(), "Unknown character '{}' at position {}", e.c, e.idx);
                },
                [&](const JsonGetError& e) {
                    std::format_to(ctx.out(), "Can't find object with the '{}' key", s_keyToStr(e.key));
                },
                [&](const JsonFindError& e) {
                    string sla{ e.currentPath
                            | std::views::transform(s_keyToStr)
                            | std::views::join_with(string_view{ ", " })
                            | ranges::to<string>() };
                    std::format_to(ctx.out(), "Unable to find '{}' in the tree [{}]", s_keyToStr(e.key), sla);
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
                        "IllFormed: unknown error, probably a invalid character",
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
                    std::format_to(ctx.out(), "ah, deu algo aí, dps mexo na Hermes: {}", to_underlying(e));
                },
                [&](const RequestBuildErrorEnum e) {
                    constexpr const char* desc[]{
                        "InvalidResponse: unknown error, probably a invalid character",
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
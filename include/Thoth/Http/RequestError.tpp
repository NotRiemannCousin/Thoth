#pragma once


template<>
struct std::formatter<Thoth::Http::RequestError> {
    using RequestError = Thoth::Http::RequestError;

    using JsonParseError = Thoth::Http::JsonParseError;
    using JsonSearchError = Thoth::Http::JsonSearchError;
    using UrlParseErrorEnum = Thoth::Http::UrlParseErrorEnum;
    using ConnectionErrorEnum = Thoth::Http::ConnectionErrorEnum;
    using RequestBuildErrorEnum = Thoth::Http::RequestBuildErrorEnum;

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
                [&](const JsonSearchError& e) {
                    string sla{ e.currentPath
                            | views::transform(s_keyToStr)
                            | views::join_with(string_view{ ", " })
                            | ranges::to<string>() };
                    std::format_to(ctx.out(), "Unable to find '{}' in the tree [{}]", s_keyToStr(e.key), sla);
                },
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
                }
            }, error
        );

        return ctx.out();
    }
};
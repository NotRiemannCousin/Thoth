#pragma once
#include <format>
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>
#include <Thoth/String/Utils.hpp>

namespace Thoth::Http::NHeaders {
    template<>
    inline std::optional<MimeType> Scan<MimeType>(std::string_view input) {
        using RfcSpec = String::CharSequences::Http; // RFC 9110

        constexpr auto s_isToken = [](std::string_view str) {
            return !str.empty() && str.find_first_not_of(RfcSpec::tchar) == std::string::npos;
        };

        String::Trim(input, RfcSpec::whitespace);

        const auto slashIdx{ input.find('/') };
        if (slashIdx == std::string_view::npos) return std::nullopt;

        const auto typeStr{ input.substr(0, slashIdx) };
        input.remove_prefix(typeStr.size() + 1);

        const auto semiIdx{ input.find_first_not_of(RfcSpec::tchar) };
        const auto subtypeStr{ input.substr(0, semiIdx) };
        input.remove_prefix(subtypeStr.size());

        if (!s_isToken(typeStr) || !s_isToken(subtypeStr)) return std::nullopt;

        MimeType mime{
            .type    = std::string{ typeStr },
            .subtype = std::string{ subtypeStr }
        };

        while (String::LeftTrim(input, RfcSpec::whitespace), !input.empty()) {
            if (input[0] != ';') return std::nullopt;
            input.remove_prefix(1);
            String::LeftTrim(input, RfcSpec::whitespace);
            if (input[0] == ';') continue;

            const auto equalIdx{ input.find('=') };
            if (equalIdx == std::string_view::npos) return std::nullopt;

            const auto key{ input.substr(0, equalIdx) };
            input.remove_prefix(key.size() + 1);

            if (input.empty() || !s_isToken(key)) return std::nullopt;

            if (input[0] == '"') {
                input.remove_prefix(1);
                std::string value;

                while (input[0] != '"') {
                    if (input.empty()) return std::nullopt;

                    if (input[0] == '\\') {
                        input.remove_prefix(1);
                        if (input.empty()) return std::nullopt;
                    }

                    if (!String::IsVisible(input[0]) && !RfcSpec::whitespace.contains(input[0])) return std::nullopt;
                    value.push_back(input[0]);
                    input.remove_prefix(1);

                    if (input.empty()) return std::nullopt;
                }
                input.remove_prefix(1);

                mime.options.emplace_back(key, std::move(value));
            }
            else {
                const auto endParam{ input.find_first_not_of(RfcSpec::tchar) };
                std::string_view value{ input.substr(0, endParam) };
                input.remove_prefix(value.size());

                if (!s_isToken(value)) return std::nullopt;

                mime.options.emplace_back(key, value);
            }
        }

        return mime;
    }
}


template<>
struct std::formatter<Thoth::Http::NHeaders::MimeType>{
    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    static auto format(const Thoth::Http::NHeaders::MimeType &mime, std::format_context &ctx) {
        std::format_to(ctx.out(), "{}/{}", mime.type, mime.subtype);

        static constexpr auto s_quoted = [](string_view str) {
            string res;
            for (const char c : str) {
                if (c == '\\' || c == '\"')
                    res += '\\';
                res += c;
            }
            return res;
        };

        for (const auto [fst, snd] : mime.options)
            if (snd.contains("\\\""))
                std::format_to(ctx.out(), ";{}=\"{}\"", fst, s_quoted(snd));
            else
                std::format_to(ctx.out(), ";{}={}", fst, snd);

        return ctx.out();
    }
};

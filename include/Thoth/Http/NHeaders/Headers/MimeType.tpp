#pragma once
#include <format>
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>
#include <Thoth/String/Utils.hpp>

template<>
struct Thoth::Utils::Scanner<Thoth::Http::NHeaders::MimeTypeHeader> {
    using MimeTypeHeader = Http::NHeaders::MimeTypeHeader;

    static bool Parse(const std::string_view str) {
        return str.empty();
    }

    std::optional<MimeTypeHeader> Scan(std::string_view input) {
        using RfcSpec = String::CharSequences::Http;

        constexpr auto isToken{ [](std::string_view str) {
            return !str.empty() && str.find_first_not_of(RfcSpec::k_tchar) == std::string::npos;
        } };

        String::Trim(input, RfcSpec::k_whitespace);
        const auto slashIdx{ input.find('/') };
        if (slashIdx == std::string_view::npos) return std::nullopt;

        const auto typeStr{ input.substr(0, slashIdx) };
        input.remove_prefix(typeStr.size() + 1);

        const auto subtypeStr{ String::TrimmedStr(input) };

        if (!isToken(typeStr) || !isToken(subtypeStr)) return std::nullopt;

        return MimeTypeHeader{
            std::string{ typeStr },
            std::string{ subtypeStr }
        };
    }
};

template<>
struct std::formatter<Thoth::Http::NHeaders::MimeTypeHeader>{
    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::NHeaders::MimeTypeHeader& mime, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "{}/{}", mime.type, mime.subtype);
    }
};
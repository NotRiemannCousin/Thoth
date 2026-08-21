#pragma once
#include <format>
#include <optional>
#include <ranges>
#include <sstream>

#include <Thoth/String/Utils.hpp>

template<>
struct std::formatter<Thoth::Http::NHeaders::SameSiteEnum> {
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::NHeaders::SameSiteEnum sameSite, FormatContext& ctx) const {
        using Thoth::Http::NHeaders::SameSiteEnum;
        switch (sameSite) {
            case SameSiteEnum::None:   return std::format_to(ctx.out(), "None");
            case SameSiteEnum::Strict: return std::format_to(ctx.out(), "Strict");
            case SameSiteEnum::Lax:    return std::format_to(ctx.out(), "Lax");
        }
        return std::format_to(ctx.out(), "None");
    }
};

template<>
struct std::formatter<Thoth::Http::NHeaders::Cookie> {
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::NHeaders::Cookie& cookie, FormatContext& ctx) const {
        auto out{ std::format_to(ctx.out(), "{}={}", cookie.name, cookie.value) };

        if (cookie.domain)      out = std::format_to(out, "; Domain={}"                           , *cookie.domain);
        if (cookie.expires)     out = std::format_to(out, "; Expires={:%a, %d %b %Y %H:%M:%S GMT}", *cookie.expires);
        if (cookie.maxAge)      out = std::format_to(out, "; Max-Age={}"                          , *cookie.maxAge);
        if (cookie.path)        out = std::format_to(out, "; Path={}"                             , *cookie.path);
        if (cookie.sameSite)    out = std::format_to(out, "; SameSite={}"                         , *cookie.sameSite);

        if (cookie.httpOnly)    out = std::format_to(out, "; HttpOnly");
        if (cookie.secure)      out = std::format_to(out, "; Secure");
        if (cookie.partitioned) out = std::format_to(out, "; Partitioned");

        return out;
    }
};


namespace Thoth::Http::NHeaders::Details_ {
    inline bool AttrKeyEq(std::string_view a, std::string_view b) {
        return std::ranges::equal(a, b, String::CaseInsensitiveCompare);
    }

    inline std::optional<Cookie::Date> ScanCookieDate(std::string_view input) {
        std::istringstream ss{ std::string{ input } };

        if (std::chrono::sys_seconds parsed;
            std::chrono::from_stream(ss, "%a, %d %b %Y %H:%M:%S GMT", parsed))
            return std::chrono::time_point_cast<Cookie::Date::duration>(parsed);

        return std::nullopt;
    }
}

template<>
struct Thoth::Utils::Scanner<Thoth::Http::NHeaders::Cookie> {
    using Cookie = Http::NHeaders::Cookie;

    static bool Parse(const std::string_view str) {
        return str.empty();
    }

    std::optional<Cookie> Scan(std::string_view input) {
        namespace vs = std::views;
        using namespace Thoth::Http::NHeaders::Details_;
        using Http::NHeaders::SameSiteEnum;

        Cookie cookie{};
        bool first{ true };

        for (const auto rawSeg : input | vs::split(';')) {
            std::string_view seg{ rawSeg.begin(), rawSeg.end() };
            String::Trim(seg);
            if (seg.empty()) continue;

            const auto eq{ seg.find('=') };

            if (first) {
                first = false;
                // First segment must be "name=value"; a Set-Cookie with no name is malformed.
                if (eq == std::string_view::npos) return std::nullopt;
                cookie.name  = std::string{ seg.substr(0, eq) };
                cookie.value = std::string{ seg.substr(eq + 1) };
                continue;
            }

            const std::string_view key{ eq == std::string_view::npos ? seg : seg.substr(0, eq) };
            const std::string_view val{ eq == std::string_view::npos ? std::string_view{} : seg.substr(eq + 1) };

            if      (AttrKeyEq(key, "domain"     )) cookie.domain = std::string{ val };
            else if (AttrKeyEq(key, "path"       )) cookie.path   = std::string{ val };
            else if (AttrKeyEq(key, "secure"     )) cookie.secure = true;
            else if (AttrKeyEq(key, "httponly"   )) cookie.httpOnly = true;
            else if (AttrKeyEq(key, "partitioned")) cookie.partitioned = true;
            else if (AttrKeyEq(key, "max-age")) {
                if (auto n{ Utils::Scan<int64_t>(val) }; n) cookie.maxAge = *n;
                else return std::nullopt;
            }
            else if (AttrKeyEq(key, "expires")) {
                if (auto d{ ScanCookieDate(val) }; d) cookie.expires = *d;
                else return std::nullopt;
            }
            else if (AttrKeyEq(key, "samesite")) {
                if      (AttrKeyEq(val, "strict")) cookie.sameSite = SameSiteEnum::Strict;
                else if (AttrKeyEq(val, "lax"   )) cookie.sameSite = SameSiteEnum::Lax;
                else if (AttrKeyEq(val, "none"  )) cookie.sameSite = SameSiteEnum::None;
                else return std::nullopt;
            }
            // Unknown attributes are ignored (RFC 6265 §5.2: forward-compat, don't reject).
        }

        if (first) return std::nullopt; // empty input, no name=value at all
        return cookie;
    }
};

static_assert(Thoth::Utils::Serializable<Thoth::Http::NHeaders::Cookie>);
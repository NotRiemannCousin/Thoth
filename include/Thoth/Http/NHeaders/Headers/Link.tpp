#pragma once
#include <format>
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>
#include <Thoth/String/Utils.hpp>

template<>
struct Thoth::Utils::Scanner<Thoth::Http::NHeaders::LinkHeader> {
    using LinkHeader = Http::NHeaders::LinkHeader;

    static bool Parse(const std::string_view str) {
        return str.empty();
    }

    std::optional<LinkHeader> Scan(std::string_view input) {
        String::Trim(input);
        if (input.size() < 2 || input.front() != '<' || input.back() != '>')
            return std::nullopt;

        input.remove_prefix(1);
        input.remove_suffix(1);

        return LinkHeader{ std::string{ input } };
    }
};

template<>
struct std::formatter<Thoth::Http::NHeaders::LinkHeader> {
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::NHeaders::LinkHeader& link, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "<{}>", link.uri);
    }
};
#pragma once
#include <format>
#include <ranges>
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>
#include <Thoth/String/Utils.hpp>

template<>
struct Thoth::Utils::Scanner<Thoth::Http::NHeaders::DispositionTypeHeader> {
    using DispositionTypeHeader = Http::NHeaders::DispositionTypeHeader;

    static bool Parse(const std::string_view str) {
        return str.empty();
    }

    std::optional<DispositionTypeHeader> Scan(std::string_view input) {
        using RfcSpec = String::CharSequences::Http;

        String::Trim(input, RfcSpec::k_whitespace);

        if (input.empty() || input.find_first_not_of(RfcSpec::k_tchar) != std::string_view::npos)
            return std::nullopt;

        return DispositionTypeHeader{ input | std::views::transform(tolower) | std::ranges::to<std::string>() };
    }
};

template<>
struct std::formatter<Thoth::Http::NHeaders::DispositionTypeHeader> {
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::NHeaders::DispositionTypeHeader& d, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "{}", d.type);
    }
};
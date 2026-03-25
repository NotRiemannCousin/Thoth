#pragma once
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>
#include <algorithm>
#include <optional>
#include <ranges>
#include <format>

#include <Thoth/String/Utils.hpp>



template<>
struct Thoth::Http::NHeaders::Scanner<Thoth::Http::NHeaders::EntityTag> {
    static bool Parse(const std::string_view str) {
        return str.empty();
    }

    std::optional<EntityTag> Scan(std::string_view input) {
        String::Trim(input);

        const bool isWeak{ input.starts_with("W/") };

        if (input.size() <= (isWeak ? 4 : 2)) return std::nullopt;

        if (isWeak)
            input.remove_prefix(2);

        if (input.front() != '"' || input.back() != '"') return std::nullopt;

        input.remove_prefix(1);
        input.remove_suffix(1);

        return EntityTag{ std::string{ input }, isWeak };
    }
};

template<>
struct std::formatter<Thoth::Http::NHeaders::EntityTag> {
    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::NHeaders::EntityTag& entityTag, FormatContext& ctx) const {

        std::format_to(ctx.out(), "{}\"{}\"", entityTag.isWeak ? "W/" : "", entityTag.tag);

        return ctx.out();
    }
};

namespace Thoth::Http::NHeaders {
    static_assert(Serializable<EntityTag>);
}
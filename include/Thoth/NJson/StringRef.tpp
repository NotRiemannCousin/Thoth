#pragma once
#include <format>

template<>
struct std::formatter<Thoth::NJson::StringRef>{
    static constexpr auto parse(std::format_parse_context& ctx) {
        return std::formatter<std::string_view>{}.parse(ctx);
    }

    template<class FormatContext>
    auto format(const Thoth::NJson::StringRef& ref, FormatContext& ctx) const {
        return std::formatter<std::string_view>{}.format(ref.str, ctx);
    }
};
#pragma once
#include <format>

template<>
struct std::formatter<Thoth::NJson::StringRef>{
    static auto parse(std::format_parse_context& ctx) {
        return std::formatter<std::string_view>{}.parse(ctx);
    }

    static auto format(const Thoth::NJson::StringRef& ref, std::format_context& ctx) {
        return std::formatter<std::string_view>{}.format(ref.str, ctx);
    }
};
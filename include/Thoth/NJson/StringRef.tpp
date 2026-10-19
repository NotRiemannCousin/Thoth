#pragma once
#include <format>

template<>
struct std::formatter<Thoth::NJson::StringRef>{
    static constexpr auto parse(auto& ctx) {
        return std::formatter<std::string_view>{}.parse(ctx);
    }

    auto format(const Thoth::NJson::StringRef& ref, auto& ctx) const {
        return std::formatter<std::string_view>{}.format(static_cast<string_view>(ref), ctx);
    }
};
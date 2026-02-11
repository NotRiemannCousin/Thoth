#pragma once
#include <format>

// TODO: FUTURE: Encapsulate loops for array and objects in JsonUtil? also a nested find (e.g. (json, "map", "continents", "countries"))?


namespace Thoth::NJson {

    // template<class ... T>
    //     requires (sizeof...(T) != 0) &&
    //         (std::constructible_from<Key, typename T::first_type>,...) &&
    //         (std::constructible_from<Json, typename T::second_type>,...)
    // JsonObject::JsonObject(T &&...pairs) {
    //     // std::
    // }

    namespace detail {
        using OutIt =  std::format_context::iterator;

        void FormatJsonObj(const JsonObject& json, bool pretty, const std::string& indent, size_t indentDepth, OutIt& it, std::string& tempOutBuffer);
    }
}

template<>
struct std::formatter<Thoth::NJson::JsonObject> {
    bool pretty{};
    size_t indentLevel{};

    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end()) {
            ++it;
            pretty = true;
            auto [_, err]{ std::from_chars(
                &*ctx.begin(),
                &*ctx.begin() + std::distance(ctx.begin(), ctx.end()),
                indentLevel
            ) };

            if (err != std::errc{})
                throw std::format_error("Invalid format specifier for JsonVal");
        }
        return it;
    }

    auto format(const Thoth::NJson::JsonObject& json, std::format_context& ctx) const {
        auto it{ ctx.out() };
        std::string tempOutBuffer;

        Thoth::NJson::detail::FormatJsonObj(json, pretty, std::string(indentLevel, ' '),
            0, it, tempOutBuffer);

        return it;
    }
};
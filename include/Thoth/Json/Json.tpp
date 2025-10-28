#pragma once
#include <Thoth/Utils/FirstConvertibleVariant.hpp>
#include <Thoth/Utils/Overloads.hpp>

// TODO: FUTURE: Encapsulate loops for array and objects in JsonUtil? also a nested find (e.g. Nested(json, "map", "continents", "countries"))?

namespace Thoth::Json {
    template<class T>
    CONSTEXPR_WHEN_MSVC_STARTS_WORKING Json::JsonVal::JsonVal(T newValue)
            : value(static_cast<Utils::FirstConvertibleVariant<T, Value>>(newValue)) { }

    template<class T>
    CONSTEXPR_WHEN_MSVC_STARTS_WORKING bool Json::JsonVal::IsOf() const {
        return std::holds_alternative<T>(value);
    }
    template<class T>
    CONSTEXPR_WHEN_MSVC_STARTS_WORKING T& Json::JsonVal::As() {
        return std::get<T>(value);
    }
    template<class T>
    CONSTEXPR_WHEN_MSVC_STARTS_WORKING const T& Json::JsonVal::As() const {
        return std::get<T>(value);
    }

    namespace detail {
        using OutIt =  std::format_context::iterator;

        void FormatJsonVal(const Json::JsonVal& val, bool pretty, const std::string& indent, size_t indentDepth, OutIt& it);


        inline void EscapeJsonString(const std::string& str, OutIt& it) {
            std::format_to(it, "\"");

            for (char c : str) {
                switch (c) {
                    case '"':  std::format_to(it, R"(\")"); break;
                    case '\\': std::format_to(it, R"(\\)"); break;
                    case '\b': std::format_to(it, R"(\b)"); break;
                    case '\f': std::format_to(it, R"(\f)"); break;
                    case '\n': std::format_to(it, R"(\n)"); break;
                    case '\r': std::format_to(it, R"(\r)"); break;
                    case '\t': std::format_to(it, R"(\t)"); break;
                    default:
                        if (c < 32)
                            std::format_to(it, "\\u{:04x}", static_cast<unsigned char>(c));
                        else
                            std::format_to(it, "{}", c);
                }
            }

            std::format_to(it, "\"");
        }

        inline void FormatJsonObj(const Json& json, bool pretty, const std::string& indent, size_t indentDepth, OutIt& it) {

            std::format_to(it, "{{");

                indentDepth++;
            bool first = true;
            for (const auto& [key, val] : json) {
                if (!first) std::format_to(it, ",");
                first = false;

                if (pretty) {
                    std::format_to(it, "\n");
                    for (int i{}; i < indentDepth; i++)
                        std::format_to(it, "{}", indent);
                }


                EscapeJsonString(key, it);
                std::format_to(it, ":");


                if (pretty) std::format_to(it, " ");

                FormatJsonVal(val, pretty, indent, indentDepth + 1, it);
            }
            indentDepth--;

            if (pretty && !json.Empty()) {
                std::format_to(it, "\n");
                for (int i{}; i < indentDepth; i++)
                    std::format_to(it, "{}", indent);
            }

            std::format_to(it, "}}");
        }

        inline void FormatJsonArr(const Json::Array& val, bool pretty, const std::string& indent, size_t indentDepth, OutIt& it) {
            std::format_to(it, "[");
            bool first = true;

            indentDepth++;
            for (const auto& elem : val) {
                if (!first) std::format_to(it, ",");
                first = false;

                if (pretty) {
                    std::format_to(it, "\n");
                    for (int i{}; i < indentDepth; i++)
                        std::format_to(it, "{}", indent);
                }

                FormatJsonVal(elem, pretty, indent, indentDepth, it);
            }

            if (pretty && !val.empty()) {
                std::format_to(it, "\n");
                for (int i{}; i < indentDepth; i++)
                    std::format_to(it, "{}", indent);
            }
            indentDepth--;


            if (pretty) {
                std::format_to(it, "\n");
                for (int i{}; i < indentDepth; i++)
                    std::format_to(it, "{}", indent);
            }

            std::format_to(it, "]");
        }

        inline void FormatJsonVal(const Json::JsonVal& val, bool pretty, const std::string& indent, size_t indentDepth, OutIt& it) {
            std::visit(Utils::Overloaded{
                [&](const String& str){ EscapeJsonString(str, it); },
                [&](const Number  num){ std::format_to(it, "{}", num); },
                [&](const Bool    bln){ std::format_to(it, "{}", bln); },
                [&](const Object& obj){ FormatJsonObj(*obj, pretty, indent, indentDepth, it); },
                [&](const Array&  arr){ FormatJsonArr(arr, pretty, indent, indentDepth, it); },
                [&](const Null&      ){ std::format_to(it, "null"); }
            }, val.value);
        }
    }
}


template<>
struct std::formatter<Thoth::Json::Json::JsonVal> {
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

    auto format(const Thoth::Json::Json::JsonVal& val, std::format_context& ctx) const {
        auto it = ctx.out();
        Thoth::Json::detail::FormatJsonVal(val, pretty, std::string(indentLevel, ' '), 0, it);
        return it;
    }
};

template<>
struct std::formatter<Thoth::Json::Json> {
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

    auto format(const Thoth::Json::Json& json, std::format_context& ctx) const {
        auto it{ ctx.out() };

        Thoth::Json::detail::FormatJsonObj(json, pretty, std::string(indentLevel, ' '), 0, it);

        return it;
    }
};
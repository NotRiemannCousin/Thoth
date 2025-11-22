#pragma once
#include <format>

#include <Thoth/Utils/LastMatchVariant.hpp>
#include <Thoth/Utils/Overloads.hpp>
#include <Thoth/NJson/JsonObject.hpp>

// TODO: FUTURE: Encapsulate loops for array and objects in JsonUtil? also a nested find (e.g. (json, "map", "continents", "countries"))?

// NOLINTNEXTLINE(*)
namespace Thoth::NJson {

    template<class T>
        requires std::floating_point<T> || std::integral<T> && (!std::same_as<T, bool>)
    Json::Json(T other) {
        _value = static_cast<Number>(other);
    }

    template<class T>
    requires std::convertible_to<T, std::string>
    Json::Json(T&& other) {
        _value = String::FromOwned(std::forward<T>(other));
    }

    template<class T>
    requires std::floating_point<T> || std::integral<T> && (!std::same_as<T, bool>)
    Json& Json::operator=(T other) {
        _value = static_cast<Number>(other);
        return *this;
    }

    template<class T>
        requires std::convertible_to<T, std::string>
    Json& Json::operator=(T&& other) {
        _value = String::FromOwned(std::forward<T>(other));
        return *this;
    }

    template<class T>
    bool Json::IsOfType(const Json& val) {
        return std::holds_alternative<T>(val._value);
    }

    template<class T>
    bool Json::IsOf() const {
        return std::holds_alternative<T>(_value);
    }

    template<class T>
    T& Json::AsType(Json& val) {
        return std::get<T>(val._value);
    }

    template<class T>
    T& Json::As() {
        return std::get<T>(_value);
    }
    template<class T>
    const T& Json::As() const {
        return std::get<T>(_value);
    }

    namespace detail {
        using OutIt =  std::format_context::iterator;

        void FormatJsonVal(const Json& val, bool pretty, const std::string& indent,
            size_t indentDepth, OutIt& it, std::string& tempOutBuffer);


        inline void EscapeJsonString(const std::string_view& str, const OutIt& it, std::string& tempOutBuffer) {
            tempOutBuffer = '"';
            tempOutBuffer.reserve(str.size());

            for (const char c : str) {
                switch (c) {
                    case '"':  tempOutBuffer += R"(\")"; break;
                    case '\\': tempOutBuffer += R"(\\)"; break;
                    case '\b': tempOutBuffer += R"(\b)"; break;
                    case '\f': tempOutBuffer += R"(\f)"; break;
                    case '\n': tempOutBuffer += R"(\n)"; break;
                    case '\r': tempOutBuffer += R"(\r)"; break;
                    case '\t': tempOutBuffer += R"(\t)"; break;
                    default:
                        if (c < 32)
                            tempOutBuffer += std::format("\\u{:04x}", static_cast<unsigned char>(c));
                        else
                            tempOutBuffer += c;
                }
            }

            tempOutBuffer += '"';
            std::format_to(it, "{}", tempOutBuffer);
        }

        inline void FormatJsonObj(const JsonObject& json, bool pretty, const std::string& indent,
            size_t indentDepth, OutIt& it, std::string& tempOutBuffer) {

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


                EscapeJsonString(key, it, tempOutBuffer);
                std::format_to(it, ":");


                if (pretty) std::format_to(it, " ");

                FormatJsonVal(val, pretty, indent, indentDepth + 1, it, tempOutBuffer);
            }
            indentDepth--;

            if (pretty && !json.Empty()) {
                std::format_to(it, "\n");
                for (int i{}; i < indentDepth; i++)
                    std::format_to(it, "{}", indent);
            }

            std::format_to(it, "}}");
        }

        inline void FormatJsonArr(const Array& val, bool pretty, const std::string& indent,
                size_t indentDepth, OutIt& it, std::string& tempOutBuffer) {
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

                FormatJsonVal(elem, pretty, indent, indentDepth, it,  tempOutBuffer);
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

        inline void FormatJsonVal(const Json& val, bool pretty, const std::string& indent,
            const size_t indentDepth, OutIt& it, std::string& tempOutBuffer) {
            std::visit(Utils::Overloaded{
                [&](const String& str){
                    str.Visit(Utils::Overloaded{
                        [&](const String::RefType  ref){ std::format_to(it, R"("{}")", ref); },
                        [&](const String::OwnType& own){ EscapeJsonString(own, it, tempOutBuffer); }
                    });
                },
                [&](const Number  num){ std::format_to(it, "{}", num); },
                [&](const Bool    bln){ std::format_to(it, "{}", bln); },
                [&](const Object& obj){ FormatJsonObj(*obj, pretty, indent, indentDepth, it, tempOutBuffer); },
                [&](const Array&  arr){ FormatJsonArr(arr, pretty, indent, indentDepth, it, tempOutBuffer); },
                [&](const Null&      ){ std::format_to(it, "null"); }
            }, static_cast<const Json::Value&>(val));
        }
    }
}


template<>
struct std::formatter<Thoth::NJson::Json> {
    bool pretty{};
    size_t indentLevel{};

    auto parse(std::format_parse_context& ctx) {
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

    auto format(const Thoth::NJson::Json& val, std::format_context& ctx) const {
        auto it = ctx.out();
        std::string tempOutBuffer;

        Thoth::NJson::detail::FormatJsonVal(val, pretty, std::string(indentLevel, ' '),
            0, it, tempOutBuffer);
        return it;
    }
};

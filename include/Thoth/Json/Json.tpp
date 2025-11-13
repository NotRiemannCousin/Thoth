#pragma once
#include <print>
#include <Thoth/Utils/LastMatchVariant.hpp>
#include <Thoth/Utils/Overloads.hpp>

// TODO: FUTURE: Encapsulate loops for array and objects in JsonUtil? also a nested find (e.g. Nested(json, "map", "continents", "countries"))?

namespace Thoth::Json {

    // template<class T>
    //     requires(!std::same_as<std::remove_cvref_t<T>, Json::JsonVal> &&
    //              std::is_constructible_v<Json::JsonVal::Value, T&&>)
    // constexpr Json::JsonVal::JsonVal(T&& newValue) : value(std::forward<T>(newValue)) { }
    //
    // template<class T>
    //     requires(!std::same_as<std::remove_cvref_t<T>, Json::JsonVal> &&
    //              std::is_constructible_v<Json::JsonVal::Value, T&&>)
    // constexpr Json::JsonVal& Json::JsonVal::operator=(T&& newValue) {
    //     value = std::forward<T>(newValue); // Atribuição direta no variant
    //     return *this;
    // }
    template<class T>
    bool Json::JsonVal::IsOfType(const JsonVal& val) {
        return std::holds_alternative<T>(val.value);
    }

    template<class T>
    bool Json::JsonVal::IsOf() const {
        return std::holds_alternative<T>(value);
    }

    template<class T>
    T& Json::JsonVal::AsType(JsonVal& val) {
        return std::get<T>(val.value);
    }
    template<class T>
    const T& Json::JsonVal::AsConstType(const JsonVal& val) {
        return std::get<T>(val.value);
    }

    template<class T>
    T& Json::JsonVal::As() {
        return std::get<T>(value);
    }
    template<class T>
    const T& Json::JsonVal::As() const {
        return std::get<T>(value);
    }


    template<std::ranges::range R>
    Json::OptRefValWrapper Json::NestedFind(R&& keys) {
        auto currJson = this;

        auto it{ keys.begin() };
        const auto itEnd{ keys.end() };
        static JsonVal dummy{ NullV };
        OptRefValWrapper child{ std::ref(dummy) };


        while (it != itEnd) {
            child = currJson->Get(*it);

            if(!child || !child->get().IsOf<Object>())
                return std::nullopt;

            currJson = &*child->get().As<Object>();
        }

        return child;
    }

    template<std::ranges::range R>
    Json::OptCRefValWrapper Json::NestedFind(R&& keys) const {
        auto currJson = this;

        auto it{ keys.begin() };
        const auto itEnd{ keys.end() };
        OptCRefValWrapper child{ NullJ };

        while (it != itEnd) {
            child = currJson->Get(*it);

            if(!child || !child->get().IsOf<Object>())
                return std::nullopt;

            currJson = &*child->get().As<Object>();
        }

        return child;
    }


    template<std::ranges::range R>
    Json::RefValWrapperOrNull Json::NestedFindOrNull(R&& keys) {
        return NestedFind(keys).value_or(NullJ);
    }

    template<std::ranges::range R>
    Json::CRefValWrapperOrNull Json::NestedFindOrNull(R&& keys) const {
        return NestedFind(keys).value_or(NullJ);
    }



    namespace detail {
        using OutIt =  std::format_context::iterator;

        void FormatJsonVal(const Json::JsonVal& val, bool pretty, const std::string& indent,
            size_t indentDepth, OutIt& it, std::string& tempOutBuffer);


        inline void EscapeJsonString(const std::string_view& str, const OutIt& it, std::string& tempOutBuffer) {
            tempOutBuffer = '"';
            tempOutBuffer.reserve(str.size());

            for (char c : str) {
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

        inline void FormatJsonObj(const Json& json, bool pretty, const std::string& indent,
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

        inline void FormatJsonArr(const Json::Array& val, bool pretty, const std::string& indent,
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

        inline void FormatJsonVal(const Json::JsonVal& val, bool pretty, const std::string& indent,
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
            }, val.value);
        }
    }
}


template<>
struct std::formatter<Thoth::Json::Json::JsonVal> {
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

    auto format(const Thoth::Json::Json::JsonVal& val, std::format_context& ctx) const {
        auto it = ctx.out();
        std::string tempOutBuffer;

        Thoth::Json::detail::FormatJsonVal(val, pretty, std::string(indentLevel, ' '),
            0, it, tempOutBuffer);
        return it;
    }
};

template<>
struct std::formatter<Thoth::Json::Json> {
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

    auto format(const Thoth::Json::Json& json, std::format_context& ctx) const {
        auto it{ ctx.out() };
        std::string tempOutBuffer;

        namespace rg = std::ranges;
        namespace vs = std::views;
        using namespace Thoth::Json;

        for (auto& val : json
                | vs::values
                | vs::filter(&Json::JsonVal::IsOfType<String>)
                // | vs::transform(&Json::JsonVal::AsConstType<String>)
                // | vs::filter(&String::IsRefType)) {
                )
                if (!val.As<String>().IsRef()) {
                std::println("porra");

                return it;
            }

        Thoth::Json::detail::FormatJsonObj(json, pretty, std::string(indentLevel, ' '),
            0, it, tempOutBuffer);

        return it;
    }
};
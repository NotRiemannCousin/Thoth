#pragma once
#include <Hermes/Utils/Overloads.hpp>
#include <ranges>


template<>
struct std::formatter<Thoth::NJson::JsonParseError> {
    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::NJson::JsonParseError err, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "Unknown character '{}' at position {}", err.c, err.idx);
    }
};


template<>
struct std::formatter<Thoth::NJson::JsonGetError> {
    static constexpr auto parse(auto &ctx) { return ctx.begin(); }


    template<class FormatContext>
    auto format(const Thoth::NJson::JsonGetError err, FormatContext& ctx) const {
        namespace vs = std::views;
        namespace rg = std::ranges;

        constexpr auto keyToStr{ [](const Thoth::NJson::Key& key) {
            return std::visit(Hermes::Utils::Overloaded{
                [](std::string objKey) { return objKey; },
                [](const int arrKey)   { return std::to_string(arrKey); }
            }, key);
        } };

        return std::format_to(ctx.out(), "Can't find object with the '{}' key", keyToStr(err.key));
    }
};


template<>
struct std::formatter<Thoth::NJson::JsonFindError> {
    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::NJson::JsonFindError err, FormatContext& ctx) const {
        namespace vs = std::views;
        namespace rg = std::ranges;

        constexpr auto keyToStr{ [](const Thoth::NJson::Key& key) {
            return std::visit(Hermes::Utils::Overloaded{
                [](std::string objKey) { return objKey; },
                [](const int arrKey)   { return std::to_string(arrKey); }
            }, key);
        } };

        std::string tree{ err.currentPath
                | vs::transform(keyToStr)
                | vs::join_with(string_view{ ", " })
                | rg::to<string>() };
        return std::format_to(ctx.out(), "Unable to find '{}' in the tree [{}]", keyToStr(err.key), tree);
    }
};


template<>
struct std::formatter<Thoth::NJson::JsonSearchError> {
    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::NJson::JsonSearchError err, FormatContext& ctx) const {
        return std::format_to(ctx.out(), "No object matches the predicate");
    }
};


template<>
struct std::formatter<Thoth::NJson::JsonWrongTypeError> {
    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::NJson::JsonWrongTypeError err, FormatContext& ctx) const {
        constexpr const char* types[] {
            "null",
            "string",
            "number",
            "bool",
            "object",
            "array"
        };

        return std::format_to(ctx.out(), "Json has the wrong type, expecting '{}' but got '{}'", types[err.idxExpected], types[err.idxGot]);
    }
};
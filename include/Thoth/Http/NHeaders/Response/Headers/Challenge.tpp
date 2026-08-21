#pragma once
#include <format>
#include <optional>
#include <Thoth/String/Utils.hpp>

template<>
struct Thoth::Utils::Scanner<Thoth::Http::NHeaders::Challenge> {
    using Challenge = Http::NHeaders::Challenge;

    static bool Parse(const std::string_view str) {
        return str.empty();
    }

    std::optional<Challenge> Scan(std::string_view input) {
        String::Trim(input);
        if (input.empty()) return std::nullopt;

        Challenge result{};
        const auto spacePos{ input.find(' ') };
        result.scheme = std::string{ input.substr(0, spacePos) };

        if (spacePos == std::string_view::npos)
            return result; // Bare scheme

        std::string_view rest{ input.substr(spacePos + 1) };
        String::Trim(rest);
        if (rest.empty()) return result;

        // Se não houver '=', ou se o '=' for no final/seguido de outro '=' (padding do Base64)
        // então é garantidamente um Token68 e não uma lista de key=value.
        const auto eqPos{ rest.find('=') };
        const bool isToken68{ eqPos == std::string_view::npos ||
                              eqPos == rest.size() - 1 ||
                              rest[eqPos + 1] == '=' };

        if (isToken68) {
            result.params.emplace_back("token68", std::string{ rest });
            return result;
        }

        std::size_t pos{ 0 };
        while (pos < rest.size()) {
            while (pos < rest.size() && (rest[pos] == ' ' || rest[pos] == ',')) ++pos;
            if (pos >= rest.size()) break;

            const auto keyStart{ pos };
            while (pos < rest.size() && rest[pos] != '=' && rest[pos] != ',') ++pos;

            std::string_view key{ rest.substr(keyStart, pos - keyStart) };
            String::Trim(key);

            if (pos >= rest.size() || rest[pos] == ',') {
                if (!key.empty())
                    result.params.emplace_back(std::string{ key }, "");
                continue;
            }

            ++pos; // Pula o '='

            std::string_view value;
            if (pos < rest.size() && rest[pos] == '"') {
                ++pos;
                const auto valStart{ pos };
                while (pos < rest.size() && rest[pos] != '"') ++pos;
                value = rest.substr(valStart, pos - valStart);
                if (pos < rest.size()) ++pos; // Pula a aspa de fechamento
            } else {
                const auto valStart{ pos };
                while (pos < rest.size() && rest[pos] != ',') ++pos;
                value = rest.substr(valStart, pos - valStart);
                String::Trim(value);
            }
            result.params.emplace_back(std::string{ key }, std::string{ value });
        }

        return result;
    }
};

template<>
struct std::formatter<Thoth::Http::NHeaders::Challenge> {
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::NHeaders::Challenge& c, FormatContext& ctx) const {
        auto out{ std::format_to(ctx.out(), "{}", c.scheme) };
        bool first{ true };

        for (const auto& [key, value] : c.params) {
            if (key == "token68") {
                out = std::format_to(out, "{}{}", first ? " " : ", ", value);
            } else {
                out = std::format_to(out, "{}{}=\"{}\"", first ? " " : ", ", key, value);
            }
            first = false;
        }
        return out;
    }
};

static_assert(Thoth::Utils::Serializable<Thoth::Http::NHeaders::Challenge>);
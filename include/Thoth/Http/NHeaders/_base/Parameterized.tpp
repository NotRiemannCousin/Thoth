#pragma once
#include <algorithm>
#include <format>
#include <optional>
#include <string>
#include <string_view>

#include <Thoth/String/Utils.hpp>
#include <Thoth/Utils/Scanner.hpp>


namespace Thoth::Http::NHeaders {
    template<Utils::Serializable T>
    bool Parameterized<T>::operator==(const Parameterized&) const = default;

    template<Utils::Serializable T>
    bool Parameterized<T>::operator==(const T& t) const {
        return value == t;
    }

    template<Utils::Serializable T>
    std::optional<std::string_view> Parameterized<T>::Param(std::string_view key) const {
        const auto it{ std::ranges::find_if(params, [&](const auto& p) {
            return std::ranges::equal(p.first, key, String::CaseInsensitiveCompare);
        }) };

        if (it == params.end()) return std::nullopt;
        return it->second;
    }

    template<Utils::Serializable T>
    Parameterized<T> Parameterized<T>::WithParam(std::string key, std::string val) const& {
        return Parameterized{ *this }.WithParam(std::move(key), std::move(val));
    }

    template<Utils::Serializable T>
    Parameterized<T> Parameterized<T>::WithParam(std::string key, std::string val) && {
        const auto it{ std::ranges::find_if(params, [&](const auto& p) {
            return std::ranges::equal(p.first, key, String::CaseInsensitiveCompare);
        }) };

        if (it != params.end())
            it->second = std::move(val);
        else
            params.emplace_back(std::move(key), std::move(val));

        return std::move(*this);
    }

    template<Utils::Serializable T>
    Parameterized<T> Parameterized<T>::WithoutParam(std::string_view key) const& {
        return Parameterized{ *this }.WithoutParam(key);
    }

    template<Utils::Serializable T>
    Parameterized<T> Parameterized<T>::WithoutParam(std::string_view key) && {
        std::erase_if(params, [&](const auto& p) {
            return std::ranges::equal(p.first, key, String::CaseInsensitiveCompare);
        });

        return std::move(*this);
    }

    template<Utils::Serializable T>
    template<class Fn>
    auto Parameterized<T>::Transform(Fn&& fn) const -> Parameterized<std::invoke_result_t<Fn, const T&>>
        requires Utils::Serializable<std::invoke_result_t<Fn, const T&>>
    {
        return { std::invoke(std::forward<Fn>(fn), value), params };
    }

    template<Utils::Serializable T>
    template<class Fn>
    auto Parameterized<T>::H_Transform(Fn fn) {
        return [fn = std::move(fn)](const Parameterized& p) { return p.Transform(fn); };
    }

    namespace details_ {
        //! @brief Parses a `;key=value` / `;key="quoted value"` tail (RFC 9110 §5.6.6). Consumes `input`.
        //! @return nullopt on malformed input; an empty vector when there simply were no parameters.
        inline std::optional<std::vector<std::pair<std::string, std::string>>> ScanParamList(std::string_view& input) {
            using RfcSpec = String::CharSequences::Http;

            constexpr auto isToken{ [](std::string_view str) {
                return !str.empty() && str.find_first_not_of(RfcSpec::k_tchar) == std::string::npos;
            } };

            std::vector<std::pair<std::string, std::string>> params;

            while (String::LeftTrim(input, RfcSpec::k_whitespace), !input.empty()) {
                if (input[0] != ';') return std::nullopt;
                input.remove_prefix(1);
                String::LeftTrim(input, RfcSpec::k_whitespace);
                if (input.empty()) return std::nullopt;

                const auto equalIdx{ input.find('=') };
                if (equalIdx == std::string_view::npos) return std::nullopt;

                const auto key{ input.substr(0, equalIdx) };
                input.remove_prefix(key.size() + 1);

                if (input.empty() || !isToken(key)) return std::nullopt;

                if (input[0] == '"') {
                    input.remove_prefix(1);
                    std::string value;

                    while (input.empty() || input[0] != '"') {
                        if (input.empty()) return std::nullopt;

                        if (input[0] == '\\') {
                            input.remove_prefix(1);
                            if (input.empty()) return std::nullopt;
                        }

                        if (!String::IsVisible(input[0]) && !RfcSpec::k_whitespace.contains(input[0])) return std::nullopt;
                        value.push_back(input[0]);
                        input.remove_prefix(1);
                    }
                    input.remove_prefix(1); // closing quote

                    params.emplace_back(std::string{ key }, std::move(value));
                } else {
                    const auto endParam{ input.find_first_not_of(RfcSpec::k_tchar) };
                    const std::string_view value{ input.substr(0, endParam) };
                    input.remove_prefix(value.size());

                    if (!isToken(value)) return std::nullopt;

                    params.emplace_back(key, value);
                }
            }

            return params;
        }

        template<class Out>
        void FormatParamList(Out& out, const std::vector<std::pair<std::string, std::string>>& params) {
            using RfcSpec = String::CharSequences::Http;

            static constexpr auto quoted{ [](std::string_view str) -> std::string {
                std::string res;
                for (const char c : str) {
                    if (c == '\\' || c == '"') res += '\\';
                    res += c;
                }
                return res;
            } };

            for (const auto& [key, val] : params) {
                if (val.find_first_not_of(RfcSpec::k_tchar) != std::string_view::npos)
                    out = std::format_to(out, ";{}=\"{}\"", key, quoted(val));
                else
                    out = std::format_to(out, ";{}={}", key, val);
            }
        }
    }
}

template<Thoth::Utils::Serializable T>
struct Thoth::Utils::Scanner<Thoth::Http::NHeaders::Parameterized<T>> {
    using Parameterized = Http::NHeaders::Parameterized<T>;

    std::string_view m_pattern{};

    bool Parse(const std::string_view str) {
        m_pattern = str;
        return true;
    }

    std::optional<Parameterized> Scan(std::string_view input) {
        String::Trim(input);
        if (input.empty()) return std::nullopt;

        // "value; params..." - split at the first top-level ';' (outside quotes).
        std::size_t splitAt{ input.size() };
        bool inQuotes{ false };
        for (std::size_t i{ 0 }; i < input.size(); ++i) {
            if (input[i] == '"') inQuotes = !inQuotes;
            else if (input[i] == ';' && !inQuotes) { splitAt = i; break; }
        }

        auto val{ Utils::Scan<T>(input.substr(0, splitAt), m_pattern) };
        if (!val) return std::nullopt;

        std::string_view paramsPart{ input.substr(splitAt) };
        auto params{ Http::NHeaders::details_::ScanParamList(paramsPart) };
        if (!params) return std::nullopt;

        return Parameterized{ std::move(*val), std::move(*params) };
    }
};

template<Thoth::Utils::Serializable T>
struct std::formatter<Thoth::Http::NHeaders::Parameterized<T>> {
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::NHeaders::Parameterized<T>& p, FormatContext& ctx) const {
        auto out{ std::format_to(ctx.out(), "{}", p.value) };
        Thoth::Http::NHeaders::details_::FormatParamList(out, p.params);
        return out;
    }
};
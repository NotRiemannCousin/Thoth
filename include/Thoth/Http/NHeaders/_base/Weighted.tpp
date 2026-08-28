#pragma once
#include <charconv>
#include <format>
#include <string>
#include <string_view>

#include <Thoth/String/Utils.hpp>
#include <Thoth/Utils/Scanner.hpp>


namespace Thoth::Http::NHeaders {
    template<Utils::Serializable T>
    bool Weighted<T>::operator==(const Weighted&) const = default;

    template<Utils::Serializable T>
    bool Weighted<T>::operator==(const T& t) const {
        return value == t;
    }

    template<Utils::Serializable T>
    Weighted<T> Weighted<T>::WithWeight(double newQ) const {
        return { value, newQ };
    }

    template<Utils::Serializable T>
    template<class Fn>
    auto Weighted<T>::Transform(Fn&& fn) const -> Weighted<std::invoke_result_t<Fn, const T&>>
        requires Utils::Serializable<std::invoke_result_t<Fn, const T&>>
    {
        return { std::invoke(std::forward<Fn>(fn), value), q };
    }

    template<Utils::Serializable T>
    template<class Fn>
    auto Weighted<T>::H_Transform(Fn fn) {
        return [fn = std::move(fn)](const Weighted& w) { return w.Transform(fn); };
    }

    namespace details_ {
        inline std::optional<double> ScanQValue(std::string_view str) {
            String::Trim(str);
            if (str.empty()) return std::nullopt;

            double q;
            const auto [ptr, ec]{ std::from_chars(str.data(), str.data() + str.size(), q) };

            if (ec != std::errc{} || ptr != str.data() + str.size()) return std::nullopt;
            if (q < 0.0 || q > 1.0) return std::nullopt; // RFC 9110 qvalue range

            return q;
        }

        //! @brief Finds and removes a top-level `;q=`/`;Q=` segment from `input`.
        //! @return The parsed weight (1.0 when absent), or nullopt if a `q=` was found but malformed.
        inline std::optional<double> ExtractWeight(std::string& input) {
            bool inQuotes{ false };

            for (std::size_t i{ 0 }; i < input.size(); ++i) {
                const char c{ input[i] };
                if (c == '"') { inQuotes = !inQuotes; continue; }
                if (inQuotes || c != ';') continue;

                std::size_t j{ i + 1 };
                while (j < input.size() && (input[j] == ' ' || input[j] == '\t')) ++j;

                if (j >= input.size() || (input[j] != 'q' && input[j] != 'Q') || j + 1 >= input.size() || input[j + 1] != '=')
                    continue;

                const std::size_t valStart{ j + 2 };
                std::size_t valEnd{ valStart };
                while (valEnd < input.size() && input[valEnd] != ';') ++valEnd;

                const auto q{ ScanQValue(std::string_view{ input }.substr(valStart, valEnd - valStart)) };
                if (!q) return std::nullopt;

                input.erase(i, valEnd - i);
                return q;
            }

            return 1.0;
        }
    }
}

template<Thoth::Utils::Serializable T>
struct Thoth::Utils::Scanner<Thoth::Http::NHeaders::Weighted<T>> {
    using Weighted = Http::NHeaders::Weighted<T>;

    std::string_view m_pattern{};

    bool Parse(const std::string_view str) {
        m_pattern = str;
        return true;
    }

    std::optional<Weighted> Scan(std::string_view input) {
        std::string remaining{ String::TrimmedStr(input) };
        if (remaining.empty()) return std::nullopt;

        const auto q{ Http::NHeaders::details_::ExtractWeight(remaining) };
        if (!q) return std::nullopt;

        if (auto val{ Utils::Scan<T>(remaining, m_pattern) }; val)
            return Weighted{ std::move(*val), *q };

        return std::nullopt;
    }
};

template<Thoth::Utils::Serializable T>
struct std::formatter<Thoth::Http::NHeaders::Weighted<T>> {
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::NHeaders::Weighted<T>& w, FormatContext& ctx) const {
        auto out{ std::format_to(ctx.out(), "{}", w.value) };

        if (w.q < 1.0)
            out = std::format_to(out, ";q={:.3f}", w.q);

        return out;
    }
};
#pragma once
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>
#include <algorithm>
#include <optional>
#include <ranges>
#include <format>

#include <Thoth/String/Utils.hpp>

// #include <Thoth/Utils/Range.hpp>
#include <Thoth/Utils/Overloads.hpp>


template<>
struct Thoth::Http::NHeaders::Scanner<Thoth::Http::NHeaders::Range> {
    static bool Parse(const std::string_view str) {
        return str.empty();
    }

    std::optional<Range> Scan(std::string_view input) {
        String::Trim(input);

        static constexpr std::string_view prefix{ "bytes=" };

        if (!input.starts_with(prefix))
            return std::nullopt;

        input.remove_prefix(prefix.size());

        if (input.empty() || (input[0] != '-' && isdigit(input[0])))
            return std::nullopt;

        if (input[0] == '-') {
            input.remove_prefix(1);
            if (auto last{ NHeaders::Scan<unsigned int>( input ) }; last)
                return SuffixedRange{ *last };
        } else {
            const auto idx{ input.find('-') };

            if (idx == std::string::npos) return std::nullopt;

            if (idx == input.length() - 1) {
                input.remove_prefix(1);
                if (const auto start{ NHeaders::Scan<unsigned int>( input ) }; start)
                    return PrefixedRange{ *start, {} };
            } else {
                const auto start{ NHeaders::Scan<unsigned int>( input.substr(0, idx) ) };
                const auto end{ NHeaders::Scan<unsigned int>( input.substr(idx) ) };

                if (start && end)
                    return PrefixedRange{ *start, *end };
            }

        }

        return std::nullopt;
    }
};

template<>
struct std::formatter<Thoth::Http::NHeaders::Range> {
    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const Thoth::Http::NHeaders::Range range, FormatContext& ctx) const {
        using namespace Thoth::Http::NHeaders;

        std::visit(
            Thoth::Utils::Overloaded{
                [&](SuffixedRange sfxRange) { std::format_to(ctx.out(), "bytes=-{}", sfxRange.last); },
                [&](PrefixedRange pfxRange) {
                    if (pfxRange.end) std::format_to(ctx.out(), "bytes={}-{}", pfxRange.start, *pfxRange.end);
                    else              std::format_to(ctx.out(), "bytes={}-"  , pfxRange.start);
                }
            },
            range
        );

        return ctx.out();
    }
};

namespace Thoth::Http::NHeaders {
    static_assert(Serializable<Range>);
}
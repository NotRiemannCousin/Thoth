#pragma once
#include <algorithm>
#include <bitset>
#include <functional>
#include <limits>
#include <ranges>

#include <Hermes/Utils/UntilMatch.hpp>

#include <Thoth/Http/NHeaders/Proxy/ValueProxy.hpp>
#include <Thoth/Http/NHeaders/Proxy/ListProxy.hpp>


namespace Thoth::Http {
    // TODO: Reinforce this constraint
    template<std::ranges::input_range R>
    std::expected<Headers, MessageParseErrorEnum> Headers::Parse(R&& headers, const size_t maxHeadersLength) {
        namespace rg = std::ranges;
        namespace vs = std::views;
        using std::string_view;
        using std::string;


        constexpr auto toLower{ [](const unsigned char c) -> char {
            if ('A' <= c && c <= 'Z')
                return c - 'A' + 'a';
            return c;
        } };
        constexpr auto isCharAllowed{ [](const unsigned char c) -> bool {
            constexpr auto allowedChars{ std::invoke([] {
                std::bitset<256> res{};

                for (char ch{'0'}; ch <= '9'; ch++) res.set(ch);
                for (char ch{'a'}; ch <= 'z'; ch++) res.set(ch);
                for (char ch{'A'}; ch <= 'Z'; ch++) res.set(ch);

                for (const char ch : "!#$%&\'*+-.^_`|~")
                    res.set(ch);

                return res;
            }) };

            return allowedChars[c];
        } };

        constexpr string_view delimiter { "\r\n" };

        string materializedHeaders;
        string_view headersView;

        if constexpr (std::constructible_from<string_view, R>) {
            const string_view input{ headers };
            if (input.size() > maxHeadersLength)
                return std::unexpected{ MessageParseErrorEnum::HeadersTooLarge };

            headersView = input;
        } else {
            const auto boundedLength{
                maxHeadersLength == std::numeric_limits<size_t>::max()
                    ? maxHeadersLength
                    : maxHeadersLength + 1
            };

            materializedHeaders = std::forward<R>(headers)
                    | vs::take(boundedLength)
                    | rg::to<string>();

            if (materializedHeaders.size() > maxHeadersLength)
                return std::unexpected{ MessageParseErrorEnum::HeadersTooLarge };

            headersView = materializedHeaders;
        }

        Headers res;

        while (!headersView.empty()) {
            auto headerRaw{ headersView | Hermes::Utils::ExclusiveUntilMatch(delimiter) };
            string headerLine{ headerRaw | rg::to<string>() };
            const auto colon{ headerLine.find(':') };

            if (colon == string::npos)
                return std::unexpected{ MessageParseErrorEnum::InvalidHeaders };

            string headerKey{ headerLine.substr(0, colon) };
            string headerVal{ headerLine.substr(colon + 1) };

            auto consumedHeader{
                headersView | Hermes::Utils::InclusiveUntilMatch(delimiter)
            };
            headersView.remove_prefix(rg::distance(consumedHeader));

            if (headerKey.empty() || headerKey.back() == ' ' || headerKey.back() == '\t'
                || !rg::all_of(headerKey, isCharAllowed))
                return std::unexpected{ MessageParseErrorEnum::InvalidHeaders };

            while (!headerVal.empty() && (headerVal.front() == ' ' || headerVal.front() == '\t'))
                headerVal.erase(headerVal.begin());

            while (!headerVal.empty() && (headerVal.back() == ' ' || headerVal.back() == '\t'))
                headerVal.pop_back();

            rg::transform(headerKey, headerKey.begin(), toLower);

            if (IsSingleValue(headerKey) && res.Exists(headerKey))
                return std::unexpected{ MessageParseErrorEnum::InvalidHeaders };

            res.Add(headerKey, headerVal);
        }

        return res;
    }


}


template<class T>
    requires (std::derived_from<T, Thoth::Http::Headers>)
struct std::formatter<T>{

    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const T& headers, FormatContext& ctx) const {
        for (const auto& p : headers.m_headers)
            format_to(ctx.out(), "{}: {}\r\n", p.first, p.second);

        return ctx.out();
    }
};

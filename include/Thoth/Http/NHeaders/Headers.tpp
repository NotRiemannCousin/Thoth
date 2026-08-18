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
    WebResult<Headers> Headers::Parse(R&& headers, const size_t maxHeadersLength) {
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
                return std::unexpected{ StatusCodeEnum::ContentTooLarge };

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
                return std::unexpected{ StatusCodeEnum::ContentTooLarge };

            headersView = materializedHeaders;
        }

        Headers res;

        while (!headersView.empty()) {
            auto headerRaw{ headersView | Hermes::Utils::ExclusiveUntilMatch(delimiter) };
            string headerLine{ headerRaw | rg::to<string>() };
            const auto colon{ headerLine.find(':') };

            if (colon == string::npos)
                return std::unexpected{ StatusCodeEnum::BadRequest };

            string headerKey{ headerLine.substr(0, colon) };
            string headerVal{ headerLine.substr(colon + 1) };

            auto consumedHeader{
                headersView | Hermes::Utils::InclusiveUntilMatch(delimiter)
            };
            headersView.remove_prefix(rg::distance(consumedHeader));

            while (!headerKey.empty() && headerKey.back() == ' ')
                headerKey.pop_back();

            while (!headerVal.empty() && headerVal.front() == ' ')
                headerVal.erase(headerVal.begin());

            while (!headerVal.empty() && headerVal.back() == ' ')
                headerVal.pop_back();

            if (headerKey.empty() || !rg::all_of(headerKey, isCharAllowed))
                return std::unexpected{ StatusCodeEnum::BadRequest };

            rg::transform(headerKey, headerKey.begin(), toLower);
            res.Add(headerKey, headerVal);
        }

        return res;
    }

    // inline auto Headers::GetSetCookieView() const {
    //     constexpr auto cmp{ [](const auto& p) {
    //         return p.first == "set-cookie";
    //     } };
    //
    //
    //     return m_headers
    //             | std::views::filter(cmp)
    //             | std::views::transform(&HeaderPair::second);
    // }

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

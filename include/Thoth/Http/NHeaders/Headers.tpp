#pragma once
#include <algorithm>
#include <bitset>
#include <functional>
#include <ranges>
#include <Hermes/Utils/UntilMatch.hpp>

#include <Thoth/Http/NHeaders/Proxy/ValueProxy.hpp>
#include <Thoth/Http/NHeaders/Proxy/ListProxy.hpp>


namespace Thoth::Http {
    // TODO: Reinforce this constraint
    template<std::ranges::input_range R>
    WebResult<Headers> Headers::Parse(R& headers, const size_t maxHeadersLength) {
        namespace rg = std::ranges;
        namespace vs = std::views;
        using std::string_view;
        using std::string;


        constexpr auto s_toLower = [](const unsigned char c) -> char {
            if ('A' <= c && c <= 'Z')
                return c - 'A' + 'a';
            return c;
        };
        constexpr auto s_isCharAllowed = [](const char c) -> bool {
            constexpr auto s_allowedChars = [] {
                std::bitset<256> res{};

                for (char ch{'0'}; ch <= '9'; ch++) res.set(ch);
                for (char ch{'a'}; ch <= 'z'; ch++) res.set(ch);
                for (char ch{'A'}; ch <= 'Z'; ch++) res.set(ch);

                for (const char ch : "!#$%&\'*+-.^_`|~")
                    res.set(ch);

                return res;
            }();

            return s_allowedChars[c];
        };

        constexpr string_view delimiter { "\r\n" };

        auto headersView{ [&] mutable {
                if constexpr (std::constructible_from<string_view, R>)
                    return string_view{ headers };
                else
                    return std::forward<R>(headers);
            }()
        };

        Headers res;

        while (true) {
            if (headersView.begin() == headersView.end())
                break;

            auto headerRaw{ headersView | Hermes::Utils::ExclusiveUntilMatch(delimiter) };

            string headerKey{ headerRaw | vs::take_while(
                    [](const char c) { return c != ':'; }) | rg::to<string>() };
            ++headerRaw.begin();
            string headerVal{ headerRaw | vs::drop_while(
                    [](const char c) { return c == ' '; }) | rg::to<string>() };


            if constexpr (std::constructible_from<string_view, R>) {
                auto broPleaseWhereIsMy_to_input_itsAlready2ndQuarterOf2026{
                    headersView | Hermes::Utils::InclusiveUntilMatch(delimiter)
                };

                headersView.remove_prefix(rg::distance(broPleaseWhereIsMy_to_input_itsAlready2ndQuarterOf2026));
            }


            while (!headerKey.empty() && headerKey.back() == ' ')
                headerKey.pop_back();

            while (!headerVal.empty() && headerVal.back() == ' ')
                headerVal.pop_back();


            if (headerKey.empty() || !rg::all_of(headerKey, s_isCharAllowed))
                return std::unexpected{ StatusCodeEnum::BadRequest };

            rg::transform(headerKey, headerKey.begin(), s_toLower);
            res.Add(headerKey, headerVal);
        }

        return res;
    }

    // inline auto Headers::GetSetCookieView() const {
    //     constexpr auto s_cmp = [](const auto& p) {
    //         return p.first == "set-cookie";
    //     };
    //
    //
    //     return _headers
    //             | std::views::filter(s_cmp)
    //             | std::views::transform(&HeaderPair::second);
    // }

}


template<class T>
    requires (std::derived_from<T, Thoth::Http::Headers>)
struct std::formatter<T>{

    static constexpr auto parse(auto &ctx) { return ctx.begin(); }

    template<class FormatContext>
    auto format(const T& headers, FormatContext& ctx) const {
        for (const auto& p : headers._headers)
            format_to(ctx.out(), "{}: {}\r\n", p.first, p.second);

        return ctx.out();
    }
};

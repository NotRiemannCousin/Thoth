#pragma once
#include <algorithm>
#include <bitset>
#include <functional>
#include <ranges>
#include <Hermes/Utils/UntilMatch.hpp>


namespace Thoth::Http {
    // TODO: Reinforce this constraint
    template<std::ranges::input_range R>
    WebResult<HttpHeaders> HttpHeaders::Parse(R& headers, const size_t maxHeadersLength) {
        namespace rg = std::ranges;
        namespace vs = std::views;
        using std::string_view;
        using std::string;


        constexpr auto toLower = [](char c) -> char {
            if ('A' <= c && c <= 'Z')
                return c - 'A' + 'a';
            return c;
        };
        constexpr auto isCharAllowed = [](const char c) -> bool {
            constexpr auto allowedChars = [] {
                std::bitset<256> res{};

                for (char ch{'0'}; ch <= '9'; ch++) res.set(ch);
                for (char ch{'a'}; ch <= 'z'; ch++) res.set(ch);
                for (char ch{'A'}; ch <= 'Z'; ch++) res.set(ch);

                for (const char ch : "!#$%&\'*+-.^_`|~")
                    res.set(ch);

                return res;
            }();

            return allowedChars[c];
        };

        constexpr string_view delimiter { "\r\n" };

        HttpHeaders res;

        while (true) {
            auto headerRaw{ headers | Hermes::Utils::UntilMatch(delimiter) };

            auto headerKey{ headerRaw | vs::take_while(
                [](const char c) { return c != ':'; }) | rg::to<string>() };

            if (headerKey == "\r\n" && (headerRaw.begin() == headerRaw.end() || *headerRaw.begin() != ':'))
                break;

            if (*headerRaw.begin() != ':')
                return std::unexpected{ HttpStatusCodeEnum::BAD_REQUEST };
            ++headerRaw.begin();

            auto headerVal{ headerRaw | vs::drop_while(
                    [](const char c) { return c == ' ' || c == '\t'; }) | rg::to<string>() };
            headerVal.pop_back(); // delimiter.size()
            headerVal.pop_back();


            while (!headerKey.empty() && headerKey.back() == ' ')
                headerKey.pop_back();

            while (!headerVal.empty() && headerVal.back() == ' ')
                headerVal.pop_back();

            if (headerKey.empty() || headerVal.empty())
                return std::unexpected{ HttpStatusCodeEnum::BAD_REQUEST };



            if (!rg::all_of(headerKey, isCharAllowed))
                return std::unexpected{ HttpStatusCodeEnum::BAD_REQUEST };

            rg::transform(headerKey, headerKey.begin(), toLower);
            res.Add(headerKey, headerVal);
        }


        return res;
    }

    inline auto HttpHeaders::GetSetCookieView() const {
        constexpr auto cmp = [](const auto& p) {
            return p.first == "set-cookie";
        };


        return _headers
                | std::views::filter(cmp)
                | std::views::transform(&HeaderPair::second);
    }

}


namespace std {
    template<>
    struct formatter<Thoth::Http::HttpHeaders>{

        static constexpr auto parse(auto &ctx) { return ctx.begin(); }

        auto format(const Thoth::Http::HttpHeaders &headers, std::format_context  &ctx) const {
            for (const auto& p : headers._headers)
                if (&p == &headers._headers.back())
                    format_to(ctx.out(), "{}: {}",     p.first, p.second);
                else
                    format_to(ctx.out(), "{}: {}\r\n", p.first, p.second);

            return ctx.out();
        }
    };
}
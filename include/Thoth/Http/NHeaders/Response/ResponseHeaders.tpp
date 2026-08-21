#pragma once

namespace Thoth::Http {
    inline auto ResponseHeaders::GetSetCookiesView() const {
        namespace vs = std::views;
        namespace rg = std::ranges;

        constexpr auto isValidCookie{ [](const auto& header) {
            return InsensitiveCmp(header.first, "set-cookie") && !String::Trimmed(header.second).empty();
        } };

        constexpr auto makePair{ [](std::string_view value) -> std::pair<std::string_view, std::string_view> {
            if (const auto semi{ value.find(';') }; semi != std::string_view::npos)
                value = value.substr(0, semi);

            if (const auto idx{ value.find('=') }; idx != std::string_view::npos)
                return { String::Trimmed(value.substr(0, idx)), String::Trimmed(value.substr(idx + 1)) };

            return { String::Trimmed(value), {} };
        } };

        constexpr auto isNotEmpty{ [](const auto& pair) {
            return !pair.first.empty();
        } };

        return m_headers
                | vs::filter(isValidCookie)
                | vs::values
                | vs::transform(String::H_ForceView())
                | vs::transform(makePair)
                | vs::filter(isNotEmpty);
    }

    template<std::ranges::input_range R>
    WebResult<ResponseHeaders> ResponseHeaders::Parse(R&& headers, size_t maxHeadersLength) {
        return Headers::Parse(std::forward<R>(headers), maxHeadersLength)
                .transform([](Headers&& header){ return ResponseHeaders{ std::move(header) }; });
    }
}
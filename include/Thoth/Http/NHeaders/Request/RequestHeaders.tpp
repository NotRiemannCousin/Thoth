#pragma once

#include <Thoth/String/Utils.hpp>

namespace Thoth::Http {
    inline auto RequestHeaders::GetCookiesView() const {
        namespace vs = std::views;
        namespace rg = std::ranges;

        constexpr auto isValidCookie{ [](auto header) {
            return header.first == "cookie" && !String::Trimmed(header.second).empty();
        } };

        constexpr auto MakePair{ [](std::string_view value) -> std::pair<std::string_view, std::string_view> {
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
                | vs::transform(vs::split(';'))
                | vs::join
                | vs::transform(String::H_ForceView())
                | vs::transform(MakePair)
                | vs::filter(isNotEmpty);
    }

    template<std::ranges::input_range R>
    WebResult<RequestHeaders> RequestHeaders::Parse(R&& headers, size_t maxHeadersLength) {
        return Headers::Parse(std::forward<R>(headers), maxHeadersLength)
                .transform([](Headers&& header){ return RequestHeaders{ std::move(header) }; });
    }
}
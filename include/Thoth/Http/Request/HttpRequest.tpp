#pragma once


namespace Thoth::Http {
    template<HttpMethodConcept Method>
    template<class T>
        requires requires (T t) { { std::format("{}", t) }; }
    std::optional<HttpRequest<Method>> HttpRequest<Method>::FromUrl(string_view url, T&& body) {
        const auto optUrl{ HttpUrl::TryDecode(url) };
        if (!optUrl)
            return std::nullopt;

        return HttpRequest{
            *optUrl,
            std::format("{}", std::forward<T>(body))
        };
    }
}
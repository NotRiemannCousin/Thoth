#pragma once


namespace Thoth::Http {
    template<HttpMethodConcept Method>
    template<class T>
        requires requires (T t) { { std::format("{}", t) }; }
    std::optional<HttpRequest<Method>> HttpRequest<Method>::FromUrl(string_view url, T&& body) {
        return HttpUrl::FromUrl(url)
            .transform([&](const auto& httpUrl) {
                return HttpRequest{  std::move(httpUrl), std::format("{}", std::forward<T>(body)) };
            });
    }
}
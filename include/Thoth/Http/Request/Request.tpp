#pragma once


namespace Thoth::Http {
    template<MethodConcept Method>
    template<class T>
        requires requires (T t) { { std::format("{}", t) }; }
    std::optional<Request<Method>> Request<Method>::FromUrl(string_view url, T&& body) {
        return Url::FromUrl(url)
            .transform([&](const auto& httpUrl) {
                return Request{  std::move(httpUrl), std::format("{}", std::forward<T>(body)) };
            });
    }
}
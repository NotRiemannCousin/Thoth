#pragma once


namespace Thoth::Http {
    template<MethodConcept Method>
    template<class T>
        requires requires (T t) { { std::format("{}", t) }; }
    std::expected<Request<Method>, RequestError> Request<Method>::FromUrl(string_view url, T&& body, Headers headers) {
        return Url::FromUrl(url)
            .transform([&](const auto& httpUrl) {
                return Request{
                    .url     = std::move(httpUrl),
                    .body    = std::format("{}", std::forward<T>(body)),
                    .headers = headers
                };
            });
    }
}
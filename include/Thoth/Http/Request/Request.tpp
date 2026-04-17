#pragma once
#include <ranges>

namespace Thoth::Http {
    template<MethodConcept Method, RequestBodyConcept Body>
    template<class T>
        requires Hermes::ByteLike<std::ranges::range_value_t<T>>
                || (std::same_as<Body, std::string> && std::formattable<T, char>)
    std::expected<Request<Method, Body>, RequestError> Request<Method, Body>::FromUrl(const std::string_view url, T&& body, Headers headers) {
        if constexpr (Hermes::ByteLike<std::ranges::range_value_t<T>>)
            return Url::FromUrl(std::string{ url })
                    .transform([&](const auto& httpUrl) {
                        return Request{
                            .url     = std::move(httpUrl),
                            .body    = std::forward<T>(body) | std::ranges::to<Body>(),
                            .headers = headers
                        };
                    });
        else
            return Url::FromUrl(std::string{ url })
                    .transform([&](const auto& httpUrl) {
                        return Request{
                            .url     = std::move(httpUrl),
                            .body    = std::format("{}", std::forward<T>(body)),
                            .headers = headers
                        };
                    });
    }
}
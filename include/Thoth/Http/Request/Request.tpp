#pragma once
#include <ranges>

namespace Thoth::Http {
    template<MethodConcept Method, ReadableBodyConcept Body>
    template<class T>
        requires Hermes::ByteLike<std::ranges::range_value_t<T>>
                || (std::same_as<Body, std::string> && std::formattable<T, char>)
    std::expected<Request<Method, Body>, ExchangeError> Request<Method, Body>::FromUrl(const std::string_view url, T&& body, Headers headers) {
        static constexpr auto makeBody{ [](T&& body) {
            if constexpr (Hermes::ByteLike<std::ranges::range_value_t<T>>)
                return std::forward<T>(body) | std::ranges::to<Body>();
            else
                return std::format("{}", std::forward<T>(body));
        } };

        return Url::FromUrl(std::string{ url })
                .transform([&](const auto& httpUrl) {
                    return Request{ { .url = std::move(httpUrl), .headers = headers }, makeBody(std::forward<T>(body)), };
                });
    }

    namespace details_ {
        template<class Stream>
        using RequestParseStage = ParseStage<Stream, RequestHead>;

        template<class Stream, WritableBodyConcept Body>
        using RequestParseCompleteStage = ParseCompleteStage<Stream, RequestHead, Body>;
    }



}

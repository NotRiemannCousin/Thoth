#pragma once
#include <ranges>

namespace Thoth::Http {
    template<MethodConcept Method, BodyConcept Body>
    template<class> requires std::same_as<Body, std::string>
    std::expected<NJson::Json, ThothError> Request<Method, Body>::AsJson() const {
        return NJson::Json::Parse(body);
    }

    template<MethodConcept Method, BodyConcept Body>
    template<class T>
        requires Hermes::ByteLike<std::ranges::range_value_t<T>> && OutgoingBodyConcept<Body>
                || (std::same_as<Body, std::string> && std::formattable<T, char>)
    std::expected<Request<Method, Body>, ThothError> Request<Method, Body>::FromUrl(const std::string_view url, T&& body, Headers headers) {
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

    template<BodyConcept TargetBody, MethodConcept Method>
    struct IntoPayload<TargetBody, Request<Method, TargetBody>> {
        Request<Method, TargetBody> val;

        IntoPayload(Request<Method, TargetBody> r) : val{ std::move(r) } {}

        operator IntoRequestPayload<TargetBody>() && {
            return IntoRequestPayload<TargetBody>{
                std::move(static_cast<RequestHead&&>(val)),
                std::move(val.body)
            };
        }
    };
}

#pragma once

namespace Thoth::NJson {
    template<class ...T>
        requires (std::constructible_from<Json::Value, T>, ...)
    Array MakeArray(T&&... ts) {
        Array arr{};
        arr.reserve(sizeof...(T));

        (arr.emplace_back(std::forward<T>(ts)), ...);

        return arr;
    }

    template<class... P>
        requires (std::constructible_from<Json, typename P::second_type> && ...)
    JsonObject MakeObject(P&&... ts) {
        return JsonObject{
            JsonObject::JsonPair(
                std::forward<P>(ts).first,
                Json{ std::forward<P>(ts).second }
            )...
        };
    }
}
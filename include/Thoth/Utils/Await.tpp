#pragma once

#include <utility>

namespace Thoth::Utils {
    template<class T>
    auto Await(T&& value) {
        if constexpr (Awaitable<T>) return std::forward<T>(value);
        else if constexpr (requires { value.value(); value.error(); }) {
            if (!value) throw std::forward<T>(value).error();
            if constexpr (std::is_void_v<decltype(value.value())>)
                return stdexec::just();
            else return stdexec::just(std::forward<T>(value).value());
        } else return stdexec::just(std::forward<T>(value));
    }
}

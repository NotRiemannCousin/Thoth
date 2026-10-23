#pragma once
#include <coroutine>
#include <concepts>
#include <stdexec/execution.hpp>

namespace Thoth::Utils {

    //! @brief Concept that defines a valid C++20 awaiter object (using member functions).
    template <class T>
    concept StandardAwaitable = requires(T awaiter, std::coroutine_handle<> handle) {
        { awaiter.await_ready() } -> std::convertible_to<bool>;
        awaiter.await_suspend(handle);
        awaiter.await_resume();
    };

    //! @brief Concept that defines a valid C++20 awaiter object (not all awaitable types).
    //! @note It's impossible to know if a value is awaitable without the promisse of the caller so it will not work
    //! for every type, but this definition is enough to this lib.
    template <class T>
    concept Awaitable = StandardAwaitable<T> || stdexec::sender<T>
        || requires(T awaiter) { { operator co_await(awaiter)  } -> StandardAwaitable; }
        || requires(T awaiter) { { awaiter.operator co_await() } -> StandardAwaitable; };


    //! @brief Normalizes "value or awaitable" into something a coroutine can co_await (converges sync and async).
    //! Used at the co_await site like `co_await Await(expr)`.
    template<class T>
    auto Await(T&& value);
}

#include <Thoth/Utils/Await.tpp>

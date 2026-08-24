#pragma once
#include <Thoth/Http/Middleware/_base.hpp>
#include <Thoth/Http/Client/Client.hpp>
#include <chrono>

namespace Thoth::Http {
    //! @brief Retries `next` up to `attempts` times with exponential backoff.
    //!
    //! Triggers on transport errors (`ConnectionErrorEnum`, `MessageParseErrorEnum`) or `5xx` status.
    //!
    //! @note Only compiles with idempotent methods, like defined by @ref MethodConcept.
    //! @tparam Next Inner handler.
    //! @param attempts Total attempts, including the first (minimum effectively 1).
    //! @param next Handler invoked on every attempt.
    //! @param baseDelay Delay before the 2nd attempt; doubles each subsequent attempt.
    //! @param respectRetryAfter Overrides backoff using the server's `Retry-After` header if present.
    //! @param maxDelay Maximum allowed wait time. Aborts retry if exceeded.
    template<class Next>
    auto Retry(uint32_t attempts, Next next, std::chrono::milliseconds baseDelay = std::chrono::milliseconds{ 100 }, bool respectRetryAfter = true, std::chrono::milliseconds maxDelay = std::chrono::minutes{ 5 });
}

#include <Thoth/Http/Middleware/Retry.tpp>

namespace Thoth::Http {
    static_assert(MiddlewareConcept<
        decltype([](auto next) { return Retry(3, std::move(next)); }),
        GetMethod, GetMethod, std::string
    >); // GetMethod is idempotent

    static_assert(!MiddlewareConcept<
        decltype([](auto next) { return Retry(3, std::move(next)); }),
        PostMethod, PostMethod, std::string
    >); // PostMethod isn't idempotent
}
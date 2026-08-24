#pragma once
#include <thread>
#include <variant>

namespace Thoth::Http {
    namespace Details_ {
        bool IsRetryable(const ThothError& error);
        bool IsRetryable(StatusCodeEnum status);

        template <class Request>
        bool IsRetryable(const ThothResult<Request>& result) {
            return result ? IsRetryable(result->status) : IsRetryable(result.error());
        }

        std::chrono::milliseconds RetryBackoff(uint32_t attempt, std::chrono::milliseconds base);
    }


    template<class Next>
    auto Retry(uint32_t attempts, Next next, std::chrono::milliseconds baseDelay, bool respectRetryAfter, std::chrono::milliseconds maxDelay) {
        namespace chr = std::chrono;

        static constexpr auto getTimeout{ Hermes::Utils::Overloaded{
            [](const chr::seconds val) {
                return chr::duration_cast<chr::milliseconds>(val);
            },
            [](const chr::utc_clock::time_point val) {
                auto diff{ val - chr::utc_clock::now() };
                return chr::duration_cast<chr::milliseconds>(std::max(diff, diff.zero()));
            }
        } };

        return [next = std::move(next), attempts, baseDelay, respectRetryAfter, maxDelay]<IdempotentMethod Method, BodyConcept Body>
            (Request<Method, Body> request) mutable -> Client::ExpResponse<Method, Body>
            requires HandlerConcept<Next, Method, Body> && std::copyable<Body>
        {
            Client::ExpResponse<Method, Body> result{ std::unexpect, GenericError{ "Retry: attempts == 0" } };

            const uint32_t total{ std::max(1u, attempts) };

            for (uint32_t attempt{}; attempt < total; ++attempt) {
                result = next(request);

                if (!Details_::IsRetryable(result) || attempt + 1 == total) break;
                auto waitTime{ Details_::RetryBackoff(attempt, baseDelay) };

                if (respectRetryAfter && result) {
                    if (auto retryAfter{ result->headers.RetryAfter().GetAsOpt() }) {
                        auto requestedWait{ std::visit(getTimeout, *retryAfter) };

                        if (requestedWait > maxDelay) break;
                        waitTime = requestedWait;
                    }
                }

                std::this_thread::sleep_for(waitTime);
            }

            return result;
        };
    }

}
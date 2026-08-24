#include <Thoth/Http/Middleware/Retry.hpp>
#include <algorithm>

namespace Thoth::Http::Details_ {
    bool IsRetryable(const ThothError& error) {
        return error.Is<ConnectionErrorEnum>() || error.Is<MessageParseErrorEnum>();
    }

    bool IsRetryable(StatusCodeEnum status) {
        return static_cast<int>(status) >= 500;
    }

    std::chrono::milliseconds RetryBackoff(unsigned attempt, std::chrono::milliseconds base) {
        //* no jitter yet
        constexpr unsigned maxShift{ 10 };
        return base * (1u << std::min(attempt, maxShift));
    }
}
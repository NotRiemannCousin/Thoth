#include <Thoth/NJson/Number.hpp>

using Thoth::NJson::Number;

std::optional<Number> Number::TryParse(const std::string_view str) noexcept {
    if (str.empty()) return std::nullopt;

    const char* first{ str.data()              };
    const char* last { str.data() + str.size() };

    const bool isFloat{ str.find('.') != std::string_view::npos
                     || str.find('e') != std::string_view::npos
                     || str.find('E') != std::string_view::npos };

    if (isFloat) {
        double val;
        const auto [ptr, ec]{ std::from_chars(first, last, val) };
        if (ec != std::errc{} || ptr != last) return std::nullopt;
        return Number{ val };
    }

    if (str.starts_with('-')) {
        int64_t val;
        const auto [ptr, ec]{ std::from_chars(first, last, val) };
        if (ec != std::errc{} || ptr != last) return std::nullopt;
        return Number{ val };
    }

    uint64_t val;
    const auto [ptr, ec]{ std::from_chars(first, last, val) };
    if (ec != std::errc{} || ptr != last) return std::nullopt;
    return Number{ val };
}


std::optional<int64_t> Number::AsI64() const noexcept {
    // 2^63: one past INT64_MAX, exactly representable as double (power of 2)
    static constexpr double k_max{  9223372036854775808.0 };
    static constexpr double k_min{ -9223372036854775808.0 }; // == INT64_MIN exactly

    return std::visit([]<class T>(const T v) -> std::optional<int64_t> {
        if constexpr (std::same_as<T, int64_t>) {
            return v;
        } else if constexpr (std::same_as<T, uint64_t>) {
            if (v > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()))
                return std::nullopt;
            return static_cast<int64_t>(v);
        } else {
            if (!std::isfinite(v) || v != std::floor(v)) return std::nullopt;
            if (v < k_min || v >= k_max)                 return std::nullopt;
            return static_cast<int64_t>(v);
        }
    }, static_cast<const std::variant<int64_t, uint64_t, double>&>(*this));
}


std::optional<uint64_t> Number::AsUI64() const noexcept {
    // 2^64: one past UINT64_MAX, exactly representable as double
    static constexpr double k_max{ 18446744073709551616.0 };

    return std::visit([]<class T>(const T v) -> std::optional<uint64_t> {
        if constexpr (std::same_as<T, int64_t>) {
            if (v < 0) return std::nullopt;
            return static_cast<uint64_t>(v);
        } else if constexpr (std::same_as<T, uint64_t>) {
            return v;
        } else {
            if (!std::isfinite(v) || v < 0.0 || v != std::floor(v)) return std::nullopt;
            if (v >= k_max)                                           return std::nullopt;
            return static_cast<uint64_t>(v);
        }
    }, static_cast<const std::variant<int64_t, uint64_t, double>&>(*this));
}


double Number::AsFloat() const noexcept {
    return std::visit([]<class T>(const T v) -> double {
        return static_cast<double>(v);
    }, static_cast<const std::variant<int64_t, uint64_t, double>&>(*this));
}


bool Number::IsIntegral() const noexcept {
    return std::holds_alternative<int64_t>(*this)
        || std::holds_alternative<uint64_t>(*this);
}

bool Number::IsFloat() const noexcept {
    return std::holds_alternative<double>(*this);
}

bool Number::IsNegative() const noexcept {
    return std::visit([]<class T>(const T v) -> bool {
        if constexpr (std::same_as<T, uint64_t>) return false;
        else if constexpr (std::same_as<T, int64_t>) return v < 0;
        else return std::signbit(v); // handles -0.0
    }, static_cast<const std::variant<int64_t, uint64_t, double>&>(*this));
}



bool Number::operator==(const Number& other) const noexcept {
    return std::visit([]<class T, class U>(const T a, const U b) -> bool {
        if constexpr (std::same_as<T, U>) {
            return a == b;
        } else if constexpr (std::same_as<T, int64_t> && std::same_as<U, uint64_t>) {
            return a >= 0 && static_cast<uint64_t>(a) == b;
        } else if constexpr (std::same_as<T, uint64_t> && std::same_as<U, int64_t>) {
            return b >= 0 && a == static_cast<uint64_t>(b);
        } else {
            return static_cast<double>(a) == static_cast<double>(b);
        }
    }, static_cast<const std::variant<int64_t, uint64_t, double>&>(*this),
       static_cast<const std::variant<int64_t, uint64_t, double>&>(other));
}
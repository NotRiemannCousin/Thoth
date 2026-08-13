#pragma once
#include <charconv>
#include <cmath>
#include <cstdint>
#include <format>
#include <limits>
#include <optional>
#include <string_view>
#include <variant>

namespace Thoth::NJson {

    //! @brief JSON number type — holds @c int64_t, @c uint64_t, or @c double.
    //!
    //! The active alternative is determined at parse time:
    //! - A literal with @c '.' or exponent (@c e / @c E) → @c double.
    //! - A literal with a leading @c '-' → @c int64_t.
    //! - Any other literal → @c uint64_t (preserves the full [0, 2⁶⁴) range).
    //!
    //! Cross-alternative equality (@c int64_t{1} vs @c uint64_t{1} vs @c double{1.0})
    //! is @e not performed — @ref operator== reflects the variant's active alternative.
    struct Number : std::variant<int64_t, uint64_t, double> {
        using variant::variant;

        //! @brief Parses a JSON number literal.
        //! @param str Raw number token — no surrounding whitespace.
        //! @return The parsed @ref Number, or @c std::nullopt on malformed input.
        [[nodiscard]] static std::optional<Number> TryParse(std::string_view str) noexcept;

        //! @brief Returns the value as @c int64_t if losslessly representable.
        //!
        //! Succeeds for @c int64_t (exact), @c uint64_t ≤ @c INT64_MAX, and finite
        //! @c double values that equal their own @c floor() within @c int64_t range.
        [[nodiscard]] std::optional<int64_t>  AsI64()  const noexcept;

        //! @brief Returns the value as @c uint64_t if losslessly representable.
        //!
        //! Succeeds for @c uint64_t (exact), non-negative @c int64_t, and finite
        //! non-negative @c double values that equal their own @c floor() within @c uint64_t range.
        [[nodiscard]] std::optional<uint64_t> AsUI64() const noexcept;

        //! @brief Returns the value as @c double. Always succeeds.
        //! @warning May lose precision for integers larger than 2⁵³.
        [[nodiscard]] double AsFloat() const noexcept;

        //! @return @c true if the held type is @c int64_t or @c uint64_t.
        [[nodiscard]] bool IsIntegral() const noexcept;
        //! @return @c true if the held type is @c double.
        [[nodiscard]] bool IsFloat()    const noexcept;
        //! @return @c true if the value is negative (including @c -0.0).
        [[nodiscard]] bool IsNegative() const noexcept;

        bool operator==(const Number& other) const noexcept;

        friend struct std::formatter<Number>;
    };
}

#include <Thoth/NJson/Number.tpp>

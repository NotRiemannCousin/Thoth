#pragma once

#include <concepts>
#include <expected>
#include <optional>
#include <type_traits>
#include <variant>

namespace Thoth::Utils {

    //! @brief `true` when `T` is exactly a `std::optional<U>`. Strip cv/ref first with `std::remove_cvref_t`.
    template<class T>
    constexpr bool IsOptional{ false };

    template<class T>
    constexpr bool IsOptional<std::optional<T>>{ true };


    //! @brief `true` when `T` is exactly a `std::expected<U, E>` (including `std::expected<void, E>`).
    template<class T>
    constexpr bool IsExpected{ false };

    template<class T, class E>
    constexpr bool IsExpected<std::expected<T, E>>{ true };


    //! @brief `true` when `T` is exactly a `std::variant<...>`.
    template<class T>
    constexpr bool IsVariant{ false };

    template<class... Ts>
    constexpr bool IsVariant<std::variant<Ts...>>{ true };


    //! @brief `true` when no type of the pack repeats, `IsUnique<>()` and `IsUnique<T>()` are `true`.
    //! @details Compares the types as written, so `int` and `const int` are different. Decay them first if needed.
    template<class... Ts>
        requires (sizeof...(Ts) == 0)
    constexpr bool IsUnique() { return true; }

    template<class First, class... Rest>
    constexpr bool IsUnique() {
        return (!std::same_as<First, Rest> && ...) && IsUnique<Rest...>();
    }


    //! @brief `std::expected<T, E>` with `void` replaced by `std::monostate`, so the value can always be stored.
    template<class T>
    using NormalizedExpectedValue = std::conditional_t<std::is_void_v<T>, std::monostate, T>;


    //! @brief Bridge between "a type" and "a value": `ToValue<V>()` gives back the non-type argument `V`, while
    //! `ToValue<T>()` gives `T{}`.
    //! @details It lets an API accept the same callable either as a template type (a stateless functor) or as a
    //! non-type argument (a function pointer, a captureless lambda stored in a `constexpr` variable), so
    //! `Fn<isBig>(x)` and `Fn(isBig, x)` end up on the same implementation.
    template<auto V>
    constexpr auto ToValue() { return V; }

    template<class T>
        requires std::default_initializable<T>
    constexpr T ToValue() { return T{}; }
}

#pragma once

#include <functional>
#include <expected>
#include <concepts>
#include <ranges>
#include <algorithm>

/*
namespace Thoth::Utils {

    //! @brief If the template parameter is a value, then returns it. If is a type then construct and returns it.
    template<auto V>
    constexpr auto ToValue() { return V; }

    //! @copybrief ToValue
    template<class T>
    constexpr T ToValue() { return T{}; }

#pragma region Const/Mut

    template<class Ret, class Class, class... Args>
    constexpr auto Const(Ret (Class::*ptr)(Args...) const) { return ptr; }
    template<class Ret, class Class, class... Args>
    constexpr auto Mut(Ret (Class::*ptr)(Args...))         { return ptr; }

    template<class Ret, class Class, class... Args>
    constexpr auto ConstFn(Ret (Class::*ptr)(Args...) const) { return std::mem_fn(ptr); }
    template<class Ret, class Class, class... Args>
    constexpr auto MutFn(Ret (Class::*ptr)(Args...))         { return std::mem_fn(ptr); }

#pragma endregion

#pragma region NulloptIf/Not

    template<bool Negate, class Pred, class Val>
        requires std::predicate<Pred&, Val&>
    constexpr std::optional<std::remove_cvref_t<Val>>
    NulloptIfImpl(Pred&& pred, Val&& value) {
        if (!TestIfImpl<Negate>(pred, value))
            return std::forward<Val>(value);
        return std::nullopt;
    }

    //! @brief In a std::optional<T> context, transforms the value to std::nullopt if the predicate match.
    template<class Pred, class Val>
    constexpr auto NulloptIf(Pred&& pred, Val&& value) {
        return NulloptIfImpl<false>(
            std::forward<Pred>(pred),
            std::forward<Val>(value)
        );
    }

    //! @copybrief NulloptIf
    template<auto Pred, class Val>
    constexpr auto NulloptIf(Val&& value) {
        return NulloptIf(ToValue<Pred>(), std::forward<Val>(value));
    }

    //! @brief In a std::optional<T> context, transforms the value to std::nullopt if the predicate *doesn't* match.
    template<class Pred, class Val>
    constexpr auto NulloptIfNot(Pred&& pred, Val&& value) {
        return NulloptIfImpl<true>(
            std::forward<Pred>(pred),
            std::forward<Val>(value)
        );
    }

    //! @copybrief NulloptIfNot
    template<auto Pred, class Val>
    constexpr auto NulloptIfNot(Val&& value) {
        return NulloptIfNot(ToValue<Pred>(), std::forward<Val>(value));
    }

    //! @hof{NulloptIf}
    template<class Pred>
    constexpr auto H_NulloptIf(Pred&& pred) {
        return [pred = std::forward<Pred>(pred)](auto&& value) {
            return NulloptIf(pred, std::forward<decltype(value)>(value));
        };
    }

    //! @hof{NulloptIf}
    template<auto Pred>
    constexpr auto H_NulloptIf() {
        return NulloptIfHof<Pred>(ToValue(Pred));
    }

    //! @hof{NulloptIfNot}
    template<class Pred>
    constexpr auto H_NulloptIfNot(Pred&& pred) {
        return [pred = std::forward<Pred>(pred)](auto&& value) {
            return NulloptIfNot(pred, std::forward<decltype(value)>(value));
        };
    }

    //! @hof{NulloptIfNot}
    template<auto Pred>
    constexpr auto H_NulloptIfNot() {
        return NulloptIfNotHof<Pred>(ToValue<Pred>());
    }

#pragma endregion

#pragma region Self

    template<class T, class F>
        requires std::invocable<F, T>
    constexpr auto Self(T&& val, F&& callable) {
        std::invoke(std::forward<F>(callable), val);
        return std::forward<T>(val);
    }

    template<class F>
    constexpr auto H_Self(F&& callable) {
        return [callable = std::forward<F>(callable)]<class T>(T&& val) {
            return Self(std::forward<F>(callable), std::forward<T>(val));
        };
    }
#pragma endregion

#pragma region Move

    inline auto H_Move() {
        return [](auto&& val) -> decltype(auto) {
            return std::move(val);
        };
    }

    inline auto H_PMove() {
        return [](auto&& ptr) -> decltype(auto) {
            return std::move(*ptr);
        };
    }

    template<class T>
    auto H_MoveAs() {
        return [](T& val) -> decltype(auto) {
            return std::move(val);
        };
    }

    template<class T>
    auto H_PMoveAs() {
        return [](T&& ptr) -> decltype(auto) {
            return std::move(*ptr);
        };
    }

#pragma endregion
}
*/

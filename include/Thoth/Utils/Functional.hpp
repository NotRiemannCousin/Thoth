#pragma once
#include <functional>
#include <concepts>


namespace Thoth::Utils {

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


#pragma region ValueOr

    template<class Opt, class Err>
    constexpr expected<typename std::remove_cvref_t<Opt>::value_type, Err> ValueOr(Opt&& val, Err&& err) {
        if (val) {
            if constexpr (std::is_rvalue_reference_v<Opt>)
                return std::move(*val);
            else
                return *val;
        }
        return std::unexpected{ std::forward<Err>(err) };
    }

    template<class Opt, class Err>
    constexpr auto ValueOrHof(Err&& err) {
        using ValueT = typename std::remove_cvref_t<Opt>::value_type;

        return [&](Opt val) -> expected<ValueT, Err> {
            if (val) {
                if constexpr (std::is_rvalue_reference_v<Opt>)
                    return std::move(*val);
                else
                    return *val;
            }
            return std::unexpected{ std::forward<Err>(err) };
        };
    }

#pragma endregion


#pragma region CallIf

    template<class P, class I, class T>
        requires std::predicate<I, T> && std::invocable<I, T>
    constexpr std::expected<T, std::invoke_result_t<I, T>> CallIf(P&& pred, I&& trans, T&& value) {
        if (std::invoke(std::forward<P&&>(pred), value))
            return std::forward<T&&>(value);
        return std::unexpected{ std::invoke(std::forward<T&&>(trans), std::forward<T&&>(value)) };
    }
    template<class P, class I, class T>
    requires std::predicate<I, T> && std::invocable<I, T>
    constexpr std::expected<T, std::invoke_result_t<I, T>> CallIf(T&& value) {
        P pred{};
        I trans{};

        if (std::invoke(std::move(pred), value))
            return std::forward<T&&>(value);
        return std::unexpected{ std::invoke( std::move(trans), std::forward<T&&>(value)) };
    }

#pragma endregion
}
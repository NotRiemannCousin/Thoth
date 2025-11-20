#pragma once
#include <functional>
#include <expected>
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
    constexpr std::expected<typename std::remove_cvref_t<Opt>::value_type, Err> ValueOr(Opt&& val, Err&& err) {
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
        using ValT = typename std::remove_cvref_t<Opt>::value_type;

        return [err = std::forward<Err>(err)](Opt val) -> std::expected<ValT, Err> {
            if (val) {
                if constexpr (std::is_rvalue_reference_v<Opt>)
                    return std::move(*val);
                else
                    return *val;
            }
            return std::unexpected{ err };
        };
    }

#pragma endregion


#pragma region CallIf

    template<class Pred, class Trans, class Val>
        requires std::predicate<Trans, Pred> && std::invocable<Trans, Val>
    constexpr std::expected<Val, std::invoke_result_t<Trans, Val>> CallIf(Pred&& pred, Trans&& trans, Val&& value) {
        if (std::invoke(std::forward<Pred>(pred), value))
            return std::forward<Val>(value);
        return std::unexpected{ std::invoke(std::forward<Val>(trans), std::forward<Val>(value)) };
    }
    template<class Pred, class Trans, class Val>
        requires std::predicate<Trans, Pred> && std::invocable<Trans, Val>
    constexpr std::expected<Val, std::invoke_result_t<Trans, Val>> CallIf(Val&& value) {
        Pred pred{};
        Trans trans{};

        if (std::invoke(std::move(pred), value))
            return std::forward<Val>(value);
        return std::unexpected{ std::invoke( std::move(trans), std::forward<Val>(value)) };
    }

#pragma endregion

#pragma region ErrorIf

    template<class Pred, class Val, class Err>
        requires std::predicate<Pred&, Val&>
    constexpr std::expected<Val, Err> ErrorIf(Pred&& pred, Val&& value, Err&& error) {
        if (!std::invoke(std::forward<Pred>(pred), value))
            return std::forward<Val>(value);
        return std::unexpected{ std::forward<Err>(error) };
    }

    template<auto Pred, class Val, class Err>
        requires std::predicate<decltype(Pred), Val&>
    constexpr std::expected<Val, Err> ErrorIf(Val&& value, Err&& error) {
        if (!std::invoke(Pred, value))
            return std::forward<Val>(value);
        return std::unexpected{ std::forward<Err>(error) };
    }


    template<auto Pred, class Val, class Err>
    constexpr auto ErrorIfHof(Err&& error) {
        using ValT = std::remove_cvref_t<Val>;
        using ErrT = std::remove_cvref_t<Err>;

        return [error = std::forward<Err>(error)](Val&& value) -> std::expected<ValT, ErrT> {
            if (!std::invoke(Pred, value))
                return std::forward<Val>(value);
            return std::unexpected{ ErrT(error) };
        };
    }

    template<class Pred, class Val, class Err>
    constexpr auto ErrorIfHof(Pred&& pred, Err&& error) {
        using ValT = std::remove_cvref_t<Val>;
        using ErrT = std::remove_cvref_t<Err>;

        return [pred = std::forward<Pred>(pred), error = std::forward<Err>(error)](Val&& value) -> std::expected<ValT, ErrT> {
            if (!std::invoke(std::move(pred), value))
                return std::forward<Val>(value);
            return std::unexpected{ ErrT(error) };
        };
    }
#pragma endregion

#pragma region ErrorIf

    template<class Pred, class Val, class Err>
        requires std::predicate<Pred&, Val&>
    constexpr std::expected<Val, Err> ErrorIfNot(Pred&& pred, Val&& value, Err&& error) {
        if (std::invoke(std::forward<Pred>(pred), value))
            return std::forward<Val>(value);
        return std::unexpected{ std::forward<Err>(error) };
    }

    template<auto Pred, class Val, class Err>
        requires std::predicate<decltype(Pred), Val&>
    constexpr std::expected<Val, Err> ErrorIfNot(Val&& value, Err&& error) {
        if (std::invoke(Pred, value))
            return std::forward<Val>(value);
        return std::unexpected{ std::forward<Err>(error) };
    }


    template<auto Pred, class Val, class Err>
    constexpr auto ErrorIfNotHof(Err&& error) {
        using ValT = std::remove_cvref_t<Val>;
        using ErrT = std::remove_cvref_t<Err>;

        return [error = std::forward<Err>(error)](Val&& value) -> std::expected<ValT, ErrT> {
            if (std::invoke(Pred, value))
                return std::forward<Val>(value);
            return std::unexpected{ ErrT(error) };
        };
    }

    template<class Pred, class Val, class Err>
    constexpr auto ErrorIfNotHof(Pred&& pred, Err&& error) {
        using ValT = std::remove_cvref_t<Val>;
        using ErrT = std::remove_cvref_t<Err>;

        return [pred = std::forward<Pred>(pred), error = std::forward<Err>(error)](Val&& value) -> std::expected<ValT, ErrT> {
            if (std::invoke(std::move(pred), value))
                return std::forward<Val>(value);
            return std::unexpected{ ErrT(error) };
        };
    }
#pragma endregion

}
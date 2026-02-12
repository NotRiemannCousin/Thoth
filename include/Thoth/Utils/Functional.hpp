#pragma once
#include <functional>
#include <expected>
#include <concepts>
#include <ranges>
#include <algorithm>


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


#pragma region ValueOr

    //! @brief Monad friendly function to transform std::expected<std::optional<T>, Err> into std::expected<T, Err> with a given Err.
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

    //! @hof{ValueOr}
    template<class Opt, class Err>
    constexpr auto ValueOrHof(Err&& err) {
        using OptT = std::optional<Opt>&&;
        using ValT = typename std::remove_cvref_t<OptT>::value_type;

        return [err = std::forward<Err>(err)](OptT val) -> std::expected<ValT, Err> {
            if (val) {
                if constexpr (std::is_rvalue_reference_v<OptT>)
                    return std::move(*val);
                else
                    return *val;
            }
            return std::unexpected{ err };
        };
    }

#pragma endregion


#pragma region CallIfError/Not

    template<bool Negate, class Pred, class Trans, class Val>
        requires std::predicate<Pred&, Val&> && std::invocable<Trans, Val>
    constexpr std::expected<Val, std::invoke_result_t<Trans, Val>>
    S_CallIfErrorImpl(Pred&& pred, Trans&& trans, Val&& value) {
        if (std::invoke(pred, value) ^ Negate)
            return std::forward<Val>(value);
        return std::unexpected{ std::invoke(trans, std::forward<Val>(value)) };
    }

    //! @brief In the std::expected<Val, Err> context, returns the value if the predicate match, or
    //! the std::unexpected with transformation applied to the error otherwise.
    template<class Pred, class Trans, class Val>
    constexpr auto CallIfError(Pred&& pred, Trans&& trans, Val&& value) {
        return S_CallIfErrorImpl<false>(
            std::forward<Pred>(pred),
            std::forward<Trans>(trans),
            std::forward<Val>(value)
        );
    }

    //! @copybrief CallIfError
    template<auto Pred, class Trans, class Val>
    constexpr auto CallIfError(Trans&& trans, Val&& val) {
        return CallIfError(ToValue<Pred>(), std::forward<Trans>(trans), std::forward<Val>(val));
    }

    //! @copybrief CallIfError
    template<auto Pred, auto Trans, class Val>
    constexpr auto CallIfError(Val&& val) {
        return CallIfError(ToValue<Pred>(), ToValue<Trans>(), std::forward<Val>(val));
    }

    //! @brief In the std::expected<Val, Err> context, returns the value if the predicate *doesn't* match, or
    //! the std::unexpected with transformation applied to the error otherwise.
    template<class Pred, class Trans, class Val>
    constexpr auto CallIfErrorNot(Pred&& pred, Trans&& trans, Val&& value) {
        return S_CallIfErrorImpl<true>(
            std::forward<Pred>(pred),
            std::forward<Trans>(trans),
            std::forward<Val>(value)
        );
    }

    //! @copybrief CallIfErrorNot
    template<auto Pred, class Trans, class Val>
    constexpr auto CallIfErrorNot(Trans&& trans, Val&& val) {
        return CallIfErrorNot(ToValue<Pred>(), std::forward<Trans>(trans), std::forward<Val>(val));
    }

    //! @copybrief CallIfErrorNot
    template<auto Pred, auto Trans, class Val>
    constexpr auto CallIfErrorNot(Val&& val) {
        return CallIfErrorNot(ToValue<Pred>(), ToValue<Trans>(), std::forward<Val>(val));
    }

    //! @hof{CallIfError}
    template<class Pred, class Trans>
    constexpr auto CallIfErrorHof(Pred&& pred, Trans&& trans) {
        return [pred = std::forward<Pred>(pred),
                trans = std::forward<Trans>(trans)](auto&& value) {
            return CallIfError(pred, trans, std::forward<decltype(value)>(value));
        };
    }

    //! @hof{CallIfError}
    template<auto Pred, class Trans>
    constexpr auto CallIfErrorHof(Trans&& trans) {
        return [trans = std::forward<Trans>(trans)](auto&& value) {
            return CallIfError(ToValue<Pred>(), trans, std::forward<decltype(value)>(value));
        };
    }

    //! @hof{CallIfError}
    template<auto Pred, auto Trans>
    constexpr auto CallIfErrorHof() {
        return CallIfErrorHof<Pred, Trans>(ToValue<Pred>(), ToValue<Trans>());
    }

    //! @hof{CallIfErrorNot}
    template<class Pred, class Trans>
    constexpr auto CallIfErrorNotHof(Pred&& pred, Trans&& trans) {
        return [pred = std::forward<Pred>(pred),
                trans = std::forward<Trans>(trans)](auto&& value) {
            return CallIfErrorNot(pred, trans, std::forward<decltype(value)>(value));
        };
    }

    //! @hof{CallIfErrorNot}
    template<auto Pred, class Trans>
    constexpr auto CallIfErrorNotHof(Trans&& trans) {
        return [trans = std::forward<Trans>(trans)](auto&& value) {
            return CallIfErrorNot(ToValue<Pred>(), trans, std::forward<decltype(value)>(value));
        };
    }

    //! @hof{CallIfErrorNot}
    template<auto Pred, auto Trans>
    constexpr auto CallIfErrorNotHof() {
        return CallIfErrorNotHof<Pred, Trans>(ToValue<Pred>(), ToValue<Trans>());
    }

#pragma endregion

#pragma region ErrorIf/Not

    template<bool Negate, class Pred, class Val>
        requires std::predicate<Pred&, Val&>
    constexpr bool S_TestIfImpl(Pred&& pred, Val& value) {
        return std::invoke(pred, value) ^ Negate;
    }

    template<bool Negate, class Pred, class Val, class Err>
        requires std::predicate<Pred&, Val&>
    constexpr std::expected<Val, std::decay_t<Err>>
    S_ErrorIfImpl(Pred&& pred, Val&& value, Err&& error) {
        if (!S_TestIf<Negate>(pred, value))
            return std::forward<Val>(value);
        return std::unexpected{ std::forward<Err>(error) };
    }

    //! @brief In a std::expected<Val, Err> context, transform an error into a value only if the predicate match.
    template<class Pred, class Val, class Err>
    constexpr auto ErrorIf(Pred&& pred, Val&& value, Err&& error) {
        return S_ErrorIfImpl<false>(
            std::forward<Pred>(pred),
            std::forward<Val>(value),
            std::forward<Err>(error)
        );
    }

    //! @copybrief ErrorIf
    template<auto Pred, class Val, class Err>
    constexpr auto ErrorIf(Val&& value, Err&& error) {
        return ErrorIf(ToValue<Pred>(), std::forward<Val>(value), std::forward<Err>(error));
    }

    //! @brief In a std::expected<Val, Err> context, transform an error into a value only if the predicate *doesn't* match.
    template<class Pred, class Val, class Err>
    constexpr auto ErrorIfNot(Pred&& pred, Val&& value, Err&& error) {
        return S_ErrorIfImpl<true>(
            std::forward<Pred>(pred),
            std::forward<Val>(value),
            std::forward<Err>(error)
        );
    }

    //! @copybrief ErrorIfNot
    template<auto Pred, class Val, class Err>
    constexpr auto ErrorIfNot(Val&& value, Err&& error) {
        return ErrorIf(ToValue<Pred>(), std::forward<Val>(value), std::forward<Err>(error));
    }

    //! @hof{ErrorIf}
    template<class Pred, class Err>
    constexpr auto ErrorIfHof(Pred&& pred, Err&& error) {
        return [pred = std::forward<Pred>(pred),
                error = std::forward<Err>(error)](auto&& value) {
            return ErrorIf(pred, std::forward<decltype(value)>(value), error);
        };
    }

    //! @hof{ErrorIf}
    template<auto Pred, class Err>
    constexpr auto ErrorIfHof(Err&& error) {
        return [error = std::forward<Err>(error)](auto&& value) {
            return ErrorIf(ToValue<Pred>(), std::forward<decltype(value)>(value), error);
        };
    }
    //! @hof{ErrorIfNof}
    template<class Pred, class Err>
    constexpr auto ErrorIfNotHof(Pred&& pred, Err&& error) {
        return [pred = std::forward<Pred>(pred),
                error = std::forward<Err>(error)](auto&& value) {
            return ErrorIfNot(pred, std::forward<decltype(value)>(value), error);
        };
    }
    //! @hof{ErrorIfNot}
    template<auto Pred, class Err>
    constexpr auto ErrorIfNotHof(Err&& error) {
        return [error = std::forward<Err>(error)](auto&& value) {
            return ErrorIfNot(ToValue<Pred>(), std::forward<decltype(value)>(value), error);
        };
    }

#pragma endregion


#pragma region Transform{Opt,Exp}If/Not

    template<bool Negate, class Pred, class Trans, class Val>
        requires std::predicate<Pred&, Val&> && std::invocable<Trans, Val>
    constexpr auto S_TransformOptIfImpl(Pred&& pred, Trans&& trans, Val&& value)
        -> std::optional<std::invoke_result_t<Trans, Val>>
    {
        if (std::invoke(pred, value) ^ Negate)
            return std::invoke(trans, std::forward<Val>(value));
        return std::nullopt;
    }

    template<bool Negate, class Pred, class Trans, class Val, class Err>
        requires std::predicate<Pred&, Val&> && std::invocable<Trans, Val>
    constexpr auto S_TransformExpIfImpl(Pred&& pred, Trans&& trans, Val&& value, Err&& error)
    -> std::expected<std::invoke_result_t<Trans, Val>, std::decay_t<Err>>
    {
        if (std::invoke(pred, value) ^ Negate)
            return std::invoke(trans, std::forward<Val>(value));
        return std::unexpected{ std::forward<Err>(error) };
    }

#pragma region std::optional

    //! @brief In the std::optional<Val> context, returns the transformation applied to the value
    //! if the predicate match, or std::nullopt otherwise.
    template<class Pred, class Trans, class Val>
        requires std::predicate<Pred&, Val&> && std::invocable<Trans, Val>
    constexpr auto TransformOptIf(Pred&& pred, Trans&& trans, Val&& value) {
        return S_TransformOptIfImpl<false>(
            std::forward<Pred>(pred),
            std::forward<Trans>(trans),
            std::forward<Val>(value)
        );
    }

    //! @copybrief TransformOptIf
    template<auto Pred, class Trans, class Val>
        requires std::predicate<decltype(Pred), Val&> && std::invocable<Trans, Val>
    constexpr auto TransformOptIf(Trans&& trans, Val&& value) {
        return TransformOptIf<Pred>(ToValue<Pred>(), std::forward<Trans>(trans), std::forward<Val>(value));
    }

    //! @copybrief TransformOptIf
    template<auto Pred, auto Trans, class Val>
        requires std::invocable<decltype(Trans), Val>
    constexpr auto TransformOptIf(Val&& value) {
        return TransformOptIf<Pred, Trans>(ToValue<Pred>(), ToValue<Trans>(), std::forward<Val>(value));
    }

    //! @brief In the std::optional<Val> context, returns the transformation applied to the value
    //! if the predicate *doesn't* match, or std::nullopt otherwise.
    template<class Pred, class Trans, class Val>
        requires std::predicate<Pred&, Val&> && std::invocable<Trans, Val>
    constexpr auto TransformOptIfNot(Pred&& pred, Trans&& trans, Val&& value) {
        return S_TransformOptIfImpl<true>(
            std::forward<Pred>(pred),
            std::forward<Trans>(trans),
            std::forward<Val>(value)
        );
    }

    //! @copybrief TransformOptIfNot
    template<auto Pred, class Trans, class Val>
        requires std::predicate<decltype(Pred), Val&> && std::invocable<Trans, Val>
    constexpr auto TransformOptIfNot(Trans&& trans, Val&& value) {
        return TransformOptIfNot<Pred>(ToValue<Pred>(), std::forward<Trans>(trans), std::forward<Val>(value));
    }

    //! @copybrief TransformOptIfNot
    template<auto Pred, auto Trans, class Val>
        requires std::invocable<decltype(Trans), Val>
    constexpr auto TransformOptIfNot(Val&& value) {
        return TransformOptIfNot<Pred, Trans>(ToValue<Pred>(), ToValue<Trans>(), std::forward<Val>(value));
    }

    //! @hof{TransformOptIf}
    template<class Pred, class Trans>
        constexpr auto TransformOptIfHof(Pred&& pred, Trans&& trans) {
        return [pred = std::forward<Pred>(pred),
                trans = std::forward<Trans>(trans)](auto&& value) {
            return TransformOptIf(pred, trans, std::forward<decltype(value)>(value));
        };
    }

    //! @hof{TransformOptIf}
    template<auto Pred, class Trans>
        constexpr auto TransformOptIfHof(Trans&& trans) {
        return [trans = std::forward<Trans>(trans)](auto&& value) {
            return TransformOptIf<Pred>(ToValue<Pred>(), trans, std::forward<decltype(value)>(value));
        };
    }

    //! @hof{TransformOptIf}
    template<auto Pred, auto Trans>
        constexpr auto TransformOptIfHof() {
        return TransformOptIfHof<Pred, Trans>(ToValue<Pred>(), ToValue<Trans>());
    }

    //! @hof{TransformOptIfNot}
    template<class Pred, class Trans>
        constexpr auto TransformOptIfNotHof(Pred&& pred, Trans&& trans) {
        return [pred = std::forward<Pred>(pred),
                trans = std::forward<Trans>(trans)](auto&& value) {
            return TransformOptIfNot(pred, trans, std::forward<decltype(value)>(value));
        };
    }

    //! @hof{TransformOptIfNot}
    template<auto Pred, class Trans>
        constexpr auto TransformOptIfNotHof(Trans&& trans) {
        return [trans = std::forward<Trans>(trans)](auto&& value) {
            return TransformOptIfNot<Pred>(ToValue<Pred>(), trans, std::forward<decltype(value)>(value));
        };
    }

    //! @hof{TransformOptIfNot}
    template<auto Pred, auto Trans>
        constexpr auto TransformOptIfNotHof() {
        return TransformOptIfNotHof<Pred, Trans>(ToValue<Pred>(), ToValue<Trans>());
    }
#pragma endregion

#pragma region std::expected

    //! @brief In the std::expected<Val, Err> context, returns the transformation applied to the value
    //! if the predicate match, or std::unexpected{ error } otherwise.
    template<class Pred, class Trans, class Val, class Err>
        requires std::predicate<Pred&, Val&> && std::invocable<Trans, Val>
    constexpr auto TransformExpIf(Pred&& pred, Trans&& trans, Val&& value, Err&& error) {
        return S_TransformExpIfImpl<false>(
            std::forward<Pred>(pred),
            std::forward<Trans>(trans),
            std::forward<Val>(value),
            std::forward<Err>(error)
        );
    }

    //! @copybrief TransformExpIf
    template<auto Pred, class Trans, class Val, class Err>
        requires std::predicate<decltype(Pred), Val&> && std::invocable<Trans, Val>
    constexpr auto TransformExpIf(Trans&& trans, Val&& value, Err&& error) {
        return S_TransformExpIfImpl<false>(
            ToValue<Pred>(),
            std::forward<Trans>(trans),
            std::forward<Val>(value),
            std::forward<Err>(error)
        );
    }

    //! @copybrief TransformExpIf
    template<auto Pred, auto Trans, class Val, class Err>
        requires std::invocable<decltype(Trans), Val>
    constexpr auto TransformExpIf(Val&& value, Err&& error) {
        return S_TransformExpIfImpl<false>(
            ToValue<Pred>(),
            ToValue<Trans>(),
            std::forward<Val>(value),
            std::forward<Err>(error)
        );
    }
    //! @brief In the std::expected<Val, Err> context, returns the transformation applied to the value
    //! if the predicate *doesn't* match, or std::unexpected{ error } otherwise.
    template<class Pred, class Trans, class Val, class Err>
        requires std::predicate<Pred&, Val&> && std::invocable<Trans, Val>
    constexpr auto TransformExpIfNot(Pred&& pred, Trans&& trans, Val&& value, Err&& error) {
        return S_TransformExpIfImpl<true>(
            std::forward<Pred>(pred),
            std::forward<Trans>(trans),
            std::forward<Val>(value),
            std::forward<Err>(error)
        );
    }

    //! @copybrief TransformExpIfNot
    template<auto Pred, class Trans, class Val, class Err>
        requires std::predicate<decltype(Pred), Val&> && std::invocable<Trans, Val>
    constexpr auto TransformExpIfNot(Trans&& trans, Val&& value, Err&& error) {
        return S_TransformExpIfImpl<true>(
            ToValue<Pred>(),
            std::forward<Trans>(trans),
            std::forward<Val>(value),
            std::forward<Err>(error)
        );
    }

    //! @copybrief TransformExpIfNot
    template<auto Pred, auto Trans, class Val, class Err>
        requires std::invocable<decltype(Trans), Val>
    constexpr auto TransformExpIfNot(Val&& value, Err&& error) {
        return S_TransformExpIfImpl<true>(
            ToValue<Pred>(),
            ToValue<Trans>(),
            std::forward<Val>(value),
            std::forward<Err>(error)
        );
    }

    //! @hof{TransformExpIf}
    template<class Pred, class Trans, class Err>
    constexpr auto TransformExpIfHof(Pred&& pred, Trans&& trans, Err&& error) {
        return [pred = std::forward<Pred>(pred),
                trans = std::forward<Trans>(trans),
                error = std::forward<Err>(error)](auto&& value) {
            return TransformExpIf(pred, trans, std::forward<decltype(value)>(value), error);
        };
    }

    //! @hof{TransformExpIf}
    template<auto Pred, class Trans, class Err>
        constexpr auto TransformExpIfHof(Trans&& trans, Err&& error) {
        return [trans = std::forward<Trans>(trans),
                error = std::forward<Err>(error)](auto&& value) {
            return TransformExpIf(ToValue<Pred>(), trans, std::forward<decltype(value)>(value), error);
        };
    }

    //! @hof{TransformExpIf}
    template<auto Pred, auto Trans, class Err>
    constexpr auto TransformExpIfHof(Err&& error) {
        return TransformExpIfHof(ToValue<Pred>(), ToValue<Trans>(), std::forward<Err>(error));
    }

    //! @hof{TransformExpIfNot}
    template<class Pred, class Trans, class Err>
    constexpr auto TransformExpIfNotHof(Pred&& pred, Trans&& trans, Err&& error) {
        return [pred = std::forward<Pred>(pred),
                trans = std::forward<Trans>(trans),
                error = std::forward<Err>(error)](auto&& value) {
            return TransformExpIfNot(pred, trans, std::forward<decltype(value)>(value), error);
        };
    }

    //! @hof{TransformExpIfNot}
    template<auto Pred, class Trans, class Err>
    constexpr auto TransformExpIfNotHof(Trans&& trans, Err&& error) {
        return [trans = std::forward<Trans>(trans),
                error = std::forward<Err>(error)](auto&& value) {
            return TransformExpIfNot(ToValue<Pred>(), trans, std::forward<decltype(value)>(value), error);
        };
    }

    //! @hof{TransformExpIfNot}
    template<auto Pred, auto Trans, class Err>
    constexpr auto TransformExpIfNotHof(Err&& error) {
        return TransformExpIfNotHof(ToValue<Pred>(), ToValue<Trans>(), std::forward<Err>(error));
    }
#pragma endregion

#pragma endregion

#pragma region NulloptIf/Not

    template<bool Negate, class Pred, class Val>
        requires std::predicate<Pred&, Val&>
    constexpr std::optional<std::remove_cvref_t<Val>>
    S_NulloptIfImpl(Pred&& pred, Val&& value) {
        if (!S_TestIf<Negate>(pred, value))
            return std::forward<Val>(value);
        return std::nullopt;
    }

    //! @brief In a std::optional<T> context, transforms the value to std::nullopt if the predicate match.
    template<class Pred, class Val>
    constexpr auto NulloptIf(Pred&& pred, Val&& value) {
        return S_NulloptIfImpl<false>(
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
        return S_NulloptIfImpl<true>(
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
    constexpr auto NulloptIfHof(Pred&& pred) {
        return [pred = std::forward<Pred>(pred)](auto&& value) {
            return NulloptIf(pred, std::forward<decltype(value)>(value));
        };
    }

    //! @hof{NulloptIf}
    template<auto Pred>
    constexpr auto NulloptIfHof() {
        return NulloptIfHof<Pred>(ToValue(Pred));
    }

    //! @hof{NulloptIfNot}
    template<class Pred>
    constexpr auto NulloptIfNotHof(Pred&& pred) {
        return [pred = std::forward<Pred>(pred)](auto&& value) {
            return NulloptIfNot(pred, std::forward<decltype(value)>(value));
        };
    }

    //! @hof{NulloptIfNot}
    template<auto Pred>
    constexpr auto NulloptIfNotHof() {
        return NulloptIfNotHof<Pred>(ToValue<Pred>());
    }

#pragma endregion

}
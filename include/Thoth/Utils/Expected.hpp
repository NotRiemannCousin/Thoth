#pragma once

#include <algorithm>
#include <concepts>
#include <expected>
#include <functional>
#include <optional>
#include <ranges>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include <Thoth/Utils/Traits.hpp>

namespace Thoth::Utils {

#pragma region Combining expecteds

    //! @brief Check  args (with first fail) and if it doesn't fail it will call `fn` with them.
    //! @details `fn` must return an `std::expected`, and only runs when every argument holds a value. But even with
    //! that arguments are evaluated outside the function so this reports the first error but doesn't skip the work
    //! the others. The error types are merged with `std::common_type`. `expected<void, E>` is safe here.
    //! @code
    //! AndThenAll(MakeUser, ParseName(json), ParseAge(json));   // expected<User, E>
    //! @endcode
    template<class Fn, class... Ts, class... Es>
        requires (sizeof...(Ts) > 0) && std::invocable<Fn, Ts...>
    constexpr auto AndThenAll(Fn&& fn, std::expected<Ts, Es>... args) {
        using InvokeRes = std::invoke_result_t<Fn, Ts...>;
        static_assert(IsExpected<InvokeRes>, "The function must return a std::expected");

        using E   = std::common_type_t<Es..., typename InvokeRes::error_type>;
        using Ret = std::expected<typename InvokeRes::value_type, E>;

        std::optional<E> first{};
        (void)((args ? true : (first.emplace(std::move(args).error()), false)) && ...);

        if (first)
            return Ret{ std::unexpect, std::move(*first) };

        auto res{ std::invoke(std::forward<Fn>(fn), *std::move(args)...) };
        if (!res)
            return Ret{ std::unexpect, std::move(res).error() };

        if constexpr (std::is_void_v<typename InvokeRes::value_type>)
            return Ret{};
        else
            return Ret{ std::move(*res) };
    }


    //! @brief Packs (with first fail) the values of args into a tuple.
    //! @details Same rules as @ref AndThenAll (all arguments are evaluated outside of the function, first fail, the
    //! error types are merged with `std::common_type`).
    //! @code
    //! const auto fields{ TupleOrFirstFail(Text(json, "title"), Text(json, "content")) };
    //! if (!fields) return fields.error();
    //! const auto& [title, content]{ *fields };
    //! @endcode
    template<class... Ts, class... Es>
        requires (sizeof...(Ts) > 0)
    constexpr auto TupleOrFirstFail(std::expected<Ts, Es>... args) {
        using E   = std::common_type_t<Es...>;
        using Ret = std::expected<std::tuple<Ts...>, E>;

        std::optional<E> first{};
        (void)((args ? true : (first.emplace(std::move(args).error()), false)) && ...);

        if (first)
            return Ret{ std::unexpect, std::move(*first) };

        return Ret{ std::tuple<Ts...>{ std::move(*args)... } };
    }


    //! @brief Range adaptor closure behind @ref Collect.
    struct CollectFn : std::ranges::range_adaptor_closure<CollectFn> {
        template<std::ranges::input_range R>
            requires IsExpected<std::ranges::range_value_t<R>>
        constexpr auto operator()(R&& range) const {
            using ExpectedT = std::ranges::range_value_t<R>;
            using ValT      = typename ExpectedT::value_type;
            using ErrT      = typename ExpectedT::error_type;
            using Ret       = std::expected<std::vector<ValT>, ErrT>;

            std::vector<ValT> vec{};
            if constexpr (std::ranges::sized_range<R>)
                vec.reserve(std::ranges::size(range));

            for (auto&& val : range) {
                if (!val)
                    return Ret{ std::unexpect, std::forward<decltype(val)>(val).error() };

                vec.push_back(*std::forward<decltype(val)>(val));
            }

            return Ret{ std::move(vec) };
        }
    };

    //! @brief Turns (with first fail) a range of `std::expected<T, E>` into an `std::expected<std::vector<T>, E>`.
    //! @code
    //! const auto tags{ array | std::views::transform(ParseTag) | collect };   // expected<vector<string>, E>
    //! @endcode
    inline constexpr CollectFn collect{};


    //! @brief Range adaptor closure behind @ref PartitionExpected.
    struct PartitionExpectedFn : std::ranges::range_adaptor_closure<PartitionExpectedFn> {
        template<std::ranges::input_range R>
            requires IsExpected<std::ranges::range_value_t<R>>
        constexpr auto operator()(R&& range) const {
            using ExpectedT = std::ranges::range_value_t<R>;
            using ValT      = typename ExpectedT::value_type;
            using ErrT      = typename ExpectedT::error_type;
            using Ret       = std::pair<std::vector<ValT>, std::vector<ErrT>>;

            std::vector<ValT> values{};
            std::vector<ErrT> errors{};
            if constexpr (std::ranges::sized_range<R>) {
                values.reserve(std::ranges::size(range));
                errors.reserve(std::ranges::size(range));
            }

            for (auto&& val : range) {
                if (val)
                    values.push_back(*std::forward<decltype(val)>(val));
                else
                    errors.push_back(std::forward<decltype(val)>(val).error());
            }

            return Ret{ std::move(values), std::move(errors) };
        }
    };

    //! @brief Turns a range of `std::expected<T, E>` into  `{ std::vector<T>, std::vector<E> }`, evaluating every value
    //! into one of these vectors.
    //! @code
    //! const auto tags{ formValues | std::views::transform(ParseValues) | partitionExpected }; // pair<vector<T>, vector<E>>
    //! @endcode
    inline constexpr PartitionExpectedFn partitionExpected{};


    //! @brief Left fold over a range of `std::expected<T, E>` with fast fail.
    //! @code
    //! FoldWhileSuccess(sizes, std::plus<>{}, std::size_t{ 0 });   // expected<size_t, E>
    //! @endcode
    template<std::ranges::input_range R, class F, class Acc>
        requires IsExpected<std::ranges::range_value_t<R>>
            && std::invocable<F&, Acc, typename std::ranges::range_value_t<R>::value_type>
            && std::convertible_to<
                std::invoke_result_t<F&, Acc, typename std::ranges::range_value_t<R>::value_type>, Acc>
    constexpr std::expected<Acc, typename std::ranges::range_value_t<R>::error_type>
    FoldWhileSuccess(R&& range, F&& callable, Acc initial) {
        using Ret = std::expected<Acc, typename std::ranges::range_value_t<R>::error_type>;

        for (auto&& val : range) {
            if (!val)
                return Ret{ std::unexpect, std::forward<decltype(val)>(val).error() };

            initial = std::invoke(callable, std::move(initial), *std::forward<decltype(val)>(val));
        }

        return Ret{ std::move(initial) };
    }

    //! @hof{FoldWhileSuccess}
    template<class F, class Acc>
    constexpr auto H_FoldWhileSuccess(F&& callable, Acc initial) {
        return [callable = std::forward<F>(callable), initial = std::move(initial)]
        <std::ranges::input_range R>(R&& range) {
            return FoldWhileSuccess(std::forward<R>(range), callable, initial);
        };
    }

#pragma endregion

#pragma region ValueOr

    //! @brief Turns a `std::optional<Val>` and `Err` into an `std::expected<Val, Err>`.
    template<class Opt, class Err>
        requires IsOptional<std::remove_cvref_t<Opt>>
    constexpr std::expected<typename std::remove_cvref_t<Opt>::value_type, std::decay_t<Err>>
    ValueOr(Opt&& val, Err&& err) {
        if (val)
            return *std::forward<Opt>(val);

        return std::unexpected{ std::forward<Err>(err) };
    }

    //! @hof{ValueOr}
    template<class T, class Err>
    constexpr auto H_ValueOr(Err&& err) {
        using ErrT = std::decay_t<Err>;

        return [err = std::forward<Err>(err)](std::optional<T> val) -> std::expected<T, ErrT> {
            if (val)
                return *std::move(val);

            return std::unexpected{ err };
        };
    }

#pragma endregion

#pragma region CallIfError/Not

    template<bool Negate, class Pred, class Trans, class Val>
        requires std::predicate<Pred&, Val&> && std::invocable<Trans, Val>
    constexpr std::expected<std::decay_t<Val>, std::remove_cvref_t<std::invoke_result_t<Trans, Val>>>
    CallIfErrorImpl(Pred&& pred, Trans&& trans, Val&& value) {
        if (static_cast<bool>(std::invoke(pred, value)) != Negate)
            return std::forward<Val>(value);

        return std::unexpected{ std::invoke(std::forward<Trans>(trans), std::forward<Val>(value)) };
    }


    //! @brief Returns the value when `pred(value)` is true, `std::unexpected{ trans(value) }` otherwise.
    template<class Pred, class Trans, class Val>
    constexpr auto CallIfError(Pred&& pred, Trans&& trans, Val&& value) {
        return CallIfErrorImpl<false>(std::forward<Pred>(pred), std::forward<Trans>(trans), std::forward<Val>(value));
    }

    //! @copybrief CallIfError
    //! @details `Pred` as a non-type argument: `CallIfError<isKnown>(trans, value)`.
    template<auto Pred, class Trans, class Val>
    constexpr auto CallIfError(Trans&& trans, Val&& value) {
        return CallIfErrorImpl<false>(ToValue<Pred>(), std::forward<Trans>(trans), std::forward<Val>(value));
    }

    //! @copybrief CallIfError
    //! @details `Pred` and `Trans` as non-type arguments: `CallIfError<isKnown, toStatus>(value)`.
    template<auto Pred, auto Trans, class Val>
    constexpr auto CallIfError(Val&& value) {
        return CallIfErrorImpl<false>(ToValue<Pred>(), ToValue<Trans>(), std::forward<Val>(value));
    }


    //! @brief Returns the value when `pred(value)` is false, `std::unexpected{ trans(value) }` otherwise.
    template<class Pred, class Trans, class Val>
    constexpr auto CallIfErrorNot(Pred&& pred, Trans&& trans, Val&& value) {
        return CallIfErrorImpl<true>(std::forward<Pred>(pred), std::forward<Trans>(trans), std::forward<Val>(value));
    }

    //! @copybrief CallIfErrorNot
    template<auto Pred, class Trans, class Val>
    constexpr auto CallIfErrorNot(Trans&& trans, Val&& value) {
        return CallIfErrorImpl<true>(ToValue<Pred>(), std::forward<Trans>(trans), std::forward<Val>(value));
    }

    //! @copybrief CallIfErrorNot
    template<auto Pred, auto Trans, class Val>
    constexpr auto CallIfErrorNot(Val&& value) {
        return CallIfErrorImpl<true>(ToValue<Pred>(), ToValue<Trans>(), std::forward<Val>(value));
    }


    //! @hof{CallIfError}
    template<class Pred, class Trans>
    constexpr auto H_CallIfError(Pred&& pred, Trans&& trans) {
        return [pred = std::forward<Pred>(pred), trans = std::forward<Trans>(trans)]<class T>(T&& value) {
            return CallIfError(pred, trans, std::forward<T>(value));
        };
    }

    //! @hof{CallIfError}
    template<auto Pred, class Trans>
    constexpr auto H_CallIfError(Trans&& trans) {
        return H_CallIfError(ToValue<Pred>(), std::forward<Trans>(trans));
    }

    //! @hof{CallIfError}
    template<auto Pred, auto Trans>
    constexpr auto H_CallIfError() {
        return H_CallIfError(ToValue<Pred>(), ToValue<Trans>());
    }


    //! @hof{CallIfErrorNot}
    template<class Pred, class Trans>
    constexpr auto H_CallIfErrorNot(Pred&& pred, Trans&& trans) {
        return [pred = std::forward<Pred>(pred), trans = std::forward<Trans>(trans)]<class T>(T&& value) {
            return CallIfErrorNot(pred, trans, std::forward<T>(value));
        };
    }

    //! @hof{CallIfErrorNot}
    template<auto Pred, class Trans>
    constexpr auto H_CallIfErrorNot(Trans&& trans) {
        return H_CallIfErrorNot(ToValue<Pred>(), std::forward<Trans>(trans));
    }

    //! @hof{CallIfErrorNot}
    template<auto Pred, auto Trans>
    constexpr auto H_CallIfErrorNot() {
        return H_CallIfErrorNot(ToValue<Pred>(), ToValue<Trans>());
    }

#pragma endregion

#pragma region ErrorIf/Not

    template<bool Negate, class Pred, class Val, class Err>
        requires std::predicate<Pred&, Val&>
    constexpr std::expected<std::decay_t<Val>, std::decay_t<Err>>
    ErrorIfImpl(Pred&& pred, Val&& value, Err&& error) {
        if (static_cast<bool>(std::invoke(pred, value)) != Negate)
            return std::unexpected{ std::forward<Err>(error) };

        return std::forward<Val>(value);
    }


    //! @brief Creates an `std::expected<Val, Err>` with the given args (success if `pred(value)` is false).
    template<class Pred, class Val, class Err>
    constexpr auto ErrorIf(Pred&& pred, Val&& value, Err&& error) {
        return ErrorIfImpl<false>(std::forward<Pred>(pred), std::forward<Val>(value), std::forward<Err>(error));
    }

    //! @copybrief ErrorIf
    //! @details `Pred` as a non-type argument: `ErrorIf<isEmpty>(value, error)`.
    template<auto Pred, class Val, class Err>
    constexpr auto ErrorIf(Val&& value, Err&& error) {
        return ErrorIfImpl<false>(ToValue<Pred>(), std::forward<Val>(value), std::forward<Err>(error));
    }

    //! @brief Creates an `std::expected<Val, Err>` with the given args (success if `pred(value)` is true).
    template<class Pred, class Val, class Err>
    constexpr auto ErrorIfNot(Pred&& pred, Val&& value, Err&& error) {
        return ErrorIfImpl<true>(std::forward<Pred>(pred), std::forward<Val>(value), std::forward<Err>(error));
    }

    //! @copybrief ErrorIfNot
    template<auto Pred, class Val, class Err>
    constexpr auto ErrorIfNot(Val&& value, Err&& error) {
        return ErrorIfImpl<true>(ToValue<Pred>(), std::forward<Val>(value), std::forward<Err>(error));
    }


    //! @hof{ErrorIf}
    template<class Pred, class Err>
    constexpr auto H_ErrorIf(Pred&& pred, Err&& error) {
        return [pred = std::forward<Pred>(pred), error = std::forward<Err>(error)]<class T>(T&& value) {
            return ErrorIf(pred, std::forward<T>(value), error);
        };
    }

    //! @hof{ErrorIf}
    template<auto Pred, class Err>
    constexpr auto H_ErrorIf(Err&& error) {
        return H_ErrorIf(ToValue<Pred>(), std::forward<Err>(error));
    }

    //! @hof{ErrorIfNot}
    template<class Pred, class Err>
    constexpr auto H_ErrorIfNot(Pred&& pred, Err&& error) {
        return [pred = std::forward<Pred>(pred), error = std::forward<Err>(error)]<class T>(T&& value) {
            return ErrorIfNot(pred, std::forward<T>(value), error);
        };
    }

    //! @hof{ErrorIfNot}
    template<auto Pred, class Err>
    constexpr auto H_ErrorIfNot(Err&& error) {
        return H_ErrorIfNot(ToValue<Pred>(), std::forward<Err>(error));
    }

#pragma endregion

#pragma region TransformExpIf/Not

    template<bool Negate, class Pred, class Trans, class Val, class Err>
        requires std::predicate<Pred&, Val&> && std::invocable<Trans, Val>
    constexpr auto TransformExpIfImpl(Pred&& pred, Trans&& trans, Val&& value, Err&& error)
            -> std::expected<std::remove_cvref_t<std::invoke_result_t<Trans, Val>>, std::decay_t<Err>> {
        if (static_cast<bool>(std::invoke(pred, value)) != Negate)
            return std::invoke(std::forward<Trans>(trans), std::forward<Val>(value));

        return std::unexpected{ std::forward<Err>(error) };
    }


    //! @brief Calls `trans(value)` if `pred(value)` is true, `std::unexpected{ error }` otherwise.
    template<class Pred, class Trans, class Val, class Err>
    constexpr auto TransformExpIf(Pred&& pred, Trans&& trans, Val&& value, Err&& error) {
        return TransformExpIfImpl<false>(
            std::forward<Pred>(pred), std::forward<Trans>(trans), std::forward<Val>(value), std::forward<Err>(error));
    }

    //! @copybrief TransformExpIf
    //! @details `Pred` as a non-type argument: `TransformExpIf<isBig>(trans, value, error)`.
    template<auto Pred, class Trans, class Val, class Err>
    constexpr auto TransformExpIf(Trans&& trans, Val&& value, Err&& error) {
        return TransformExpIfImpl<false>(
            ToValue<Pred>(), std::forward<Trans>(trans), std::forward<Val>(value), std::forward<Err>(error));
    }

    //! @copybrief TransformExpIf
    //! @details `Pred` and `Trans` as non-type arguments: `TransformExpIf<isBig, twice>(value, error)`.
    template<auto Pred, auto Trans, class Val, class Err>
    constexpr auto TransformExpIf(Val&& value, Err&& error) {
        return TransformExpIfImpl<false>(
            ToValue<Pred>(), ToValue<Trans>(), std::forward<Val>(value), std::forward<Err>(error));
    }


    //! @brief Calls `trans(value)` if `pred(value)` is false, `std::unexpected{ error }` otherwise.
    template<class Pred, class Trans, class Val, class Err>
    constexpr auto TransformExpIfNot(Pred&& pred, Trans&& trans, Val&& value, Err&& error) {
        return TransformExpIfImpl<true>(
            std::forward<Pred>(pred), std::forward<Trans>(trans), std::forward<Val>(value), std::forward<Err>(error));
    }

    //! @copybrief TransformExpIfNot
    template<auto Pred, class Trans, class Val, class Err>
    constexpr auto TransformExpIfNot(Trans&& trans, Val&& value, Err&& error) {
        return TransformExpIfImpl<true>(
            ToValue<Pred>(), std::forward<Trans>(trans), std::forward<Val>(value), std::forward<Err>(error));
    }

    //! @copybrief TransformExpIfNot
    template<auto Pred, auto Trans, class Val, class Err>
    constexpr auto TransformExpIfNot(Val&& value, Err&& error) {
        return TransformExpIfImpl<true>(
            ToValue<Pred>(), ToValue<Trans>(), std::forward<Val>(value), std::forward<Err>(error));
    }


    //! @hof{TransformExpIf}
    template<class Pred, class Trans, class Err>
    constexpr auto H_TransformExpIf(Pred&& pred, Trans&& trans, Err&& error) {
        return [pred = std::forward<Pred>(pred), trans = std::forward<Trans>(trans),
                error = std::forward<Err>(error)]<class T>(T&& value) {
            return TransformExpIf(pred, trans, std::forward<T>(value), error);
        };
    }

    //! @hof{TransformExpIf}
    template<auto Pred, class Trans, class Err>
    constexpr auto H_TransformExpIf(Trans&& trans, Err&& error) {
        return H_TransformExpIf(ToValue<Pred>(), std::forward<Trans>(trans), std::forward<Err>(error));
    }

    //! @hof{TransformExpIf}
    template<auto Pred, auto Trans, class Err>
    constexpr auto H_TransformExpIf(Err&& error) {
        return H_TransformExpIf(ToValue<Pred>(), ToValue<Trans>(), std::forward<Err>(error));
    }


    //! @hof{TransformExpIfNot}
    template<class Pred, class Trans, class Err>
    constexpr auto H_TransformExpIfNot(Pred&& pred, Trans&& trans, Err&& error) {
        return [pred = std::forward<Pred>(pred), trans = std::forward<Trans>(trans),
                error = std::forward<Err>(error)]<class T>(T&& value) {
            return TransformExpIfNot(pred, trans, std::forward<T>(value), error);
        };
    }

    //! @hof{TransformExpIfNot}
    template<auto Pred, class Trans, class Err>
    constexpr auto H_TransformExpIfNot(Trans&& trans, Err&& error) {
        return H_TransformExpIfNot(ToValue<Pred>(), std::forward<Trans>(trans), std::forward<Err>(error));
    }

    //! @hof{TransformExpIfNot}
    template<auto Pred, auto Trans, class Err>
    constexpr auto H_TransformExpIfNot(Err&& error) {
        return H_TransformExpIfNot(ToValue<Pred>(), ToValue<Trans>(), std::forward<Err>(error));
    }

#pragma endregion

}

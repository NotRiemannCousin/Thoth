#pragma once

#include <concepts>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>

#include <Thoth/Utils/Traits.hpp>

namespace Thoth::Utils {

#pragma region TransformOptIf

    template<bool Negate, class Pred, class Trans, class Val>
        requires std::predicate<Pred&, Val&> && std::invocable<Trans, Val>
    constexpr auto TransformOptIfImpl(Pred&& pred, Trans&& trans, Val&& value)
            -> std::optional<std::remove_cvref_t<std::invoke_result_t<Trans, Val>>> {
        if (static_cast<bool>(std::invoke(pred, value)) != Negate)
            return std::invoke(std::forward<Trans>(trans), std::forward<Val>(value));

        return std::nullopt;
    }


    //! @brief Returns `trans(value)` when `pred(value)` is true, otherwise `std::nullopt`.
    template<class Pred, class Trans, class Val>
    constexpr auto TransformOptIf(Pred&& pred, Trans&& trans, Val&& value) {
        return TransformOptIfImpl<false>(
            std::forward<Pred>(pred), std::forward<Trans>(trans), std::forward<Val>(value));
    }

    //! @copybrief TransformOptIf
    template<auto Pred, class Trans, class Val>
    constexpr auto TransformOptIf(Trans&& trans, Val&& value) {
        return TransformOptIfImpl<false>(ToValue<Pred>(), std::forward<Trans>(trans), std::forward<Val>(value));
    }

    //! @copybrief TransformOptIf
    template<auto Pred, auto Trans, class Val>
    constexpr auto TransformOptIf(Val&& value) {
        return TransformOptIfImpl<false>(ToValue<Pred>(), ToValue<Trans>(), std::forward<Val>(value));
    }


    //! @brief Returns `trans(value)` when `pred(value)` is false, otherwise `std::nullopt`.
    template<class Pred, class Trans, class Val>
    constexpr auto TransformOptIfNot(Pred&& pred, Trans&& trans, Val&& value) {
        return TransformOptIfImpl<true>(
            std::forward<Pred>(pred), std::forward<Trans>(trans), std::forward<Val>(value));
    }

    //! @copybrief TransformOptIfNot
    template<auto Pred, class Trans, class Val>
    constexpr auto TransformOptIfNot(Trans&& trans, Val&& value) {
        return TransformOptIfImpl<true>(ToValue<Pred>(), std::forward<Trans>(trans), std::forward<Val>(value));
    }

    //! @copybrief TransformOptIfNot
    template<auto Pred, auto Trans, class Val>
    constexpr auto TransformOptIfNot(Val&& value) {
        return TransformOptIfImpl<true>(ToValue<Pred>(), ToValue<Trans>(), std::forward<Val>(value));
    }


    //! @hof{TransformOptIf}
    template<class Pred, class Trans>
    constexpr auto H_TransformOptIf(Pred&& pred, Trans&& trans) {
        return [pred = std::forward<Pred>(pred), trans = std::forward<Trans>(trans)]<class T>(T&& value) {
            return TransformOptIf(pred, trans, std::forward<T>(value));
        };
    }

    //! @hof{TransformOptIf}
    template<auto Pred, class Trans>
    constexpr auto H_TransformOptIf(Trans&& trans) {
        return H_TransformOptIf(ToValue<Pred>(), std::forward<Trans>(trans));
    }

    //! @hof{TransformOptIf}
    template<auto Pred, auto Trans>
    constexpr auto H_TransformOptIf() {
        return H_TransformOptIf(ToValue<Pred>(), ToValue<Trans>());
    }


    //! @hof{TransformOptIfNot}
    template<class Pred, class Trans>
    constexpr auto H_TransformOptIfNot(Pred&& pred, Trans&& trans) {
        return [pred = std::forward<Pred>(pred), trans = std::forward<Trans>(trans)]<class T>(T&& value) {
            return TransformOptIfNot(pred, trans, std::forward<T>(value));
        };
    }

    //! @hof{TransformOptIfNot}
    template<auto Pred, class Trans>
    constexpr auto H_TransformOptIfNot(Trans&& trans) {
        return H_TransformOptIfNot(ToValue<Pred>(), std::forward<Trans>(trans));
    }

    //! @hof{TransformOptIfNot}
    template<auto Pred, auto Trans>
    constexpr auto H_TransformOptIfNot() {
        return H_TransformOptIfNot(ToValue<Pred>(), ToValue<Trans>());
    }

#pragma endregion

#pragma region NulloptIf

    template<bool Negate, class Pred, class Val>
        requires std::predicate<Pred&, Val&>
    constexpr std::optional<std::decay_t<Val>> NulloptIfImpl(Pred&& pred, Val&& value) {
        if (static_cast<bool>(std::invoke(pred, value)) != Negate)
            return std::nullopt;

        return std::forward<Val>(value);
    }


    //! @brief `std::nullopt` when `pred(value)` is true, the value itself otherwise.
    template<class Pred, class Val>
    constexpr auto NulloptIf(Pred&& pred, Val&& value) {
        return NulloptIfImpl<false>(std::forward<Pred>(pred), std::forward<Val>(value));
    }

    //! @copybrief NulloptIf
    //! @details `Pred` as a non-type argument: `NulloptIf<isEmpty>(value)`.
    template<auto Pred, class Val>
    constexpr auto NulloptIf(Val&& value) {
        return NulloptIfImpl<false>(ToValue<Pred>(), std::forward<Val>(value));
    }

    //! @brief `std::nullopt` when `pred(value)` is false, the value itself otherwise.
    template<class Pred, class Val>
    constexpr auto NulloptIfNot(Pred&& pred, Val&& value) {
        return NulloptIfImpl<true>(std::forward<Pred>(pred), std::forward<Val>(value));
    }

    //! @copybrief NulloptIfNot
    template<auto Pred, class Val>
    constexpr auto NulloptIfNot(Val&& value) {
        return NulloptIfImpl<true>(ToValue<Pred>(), std::forward<Val>(value));
    }


    //! @hof{NulloptIf}
    template<class Pred>
    constexpr auto H_NulloptIf(Pred&& pred) {
        return [pred = std::forward<Pred>(pred)]<class T>(T&& value) {
            return NulloptIf(pred, std::forward<T>(value));
        };
    }

    //! @hof{NulloptIf}
    template<auto Pred>
    constexpr auto H_NulloptIf() {
        return H_NulloptIf(ToValue<Pred>());
    }

    //! @hof{NulloptIfNot}
    template<class Pred>
    constexpr auto H_NulloptIfNot(Pred&& pred) {
        return [pred = std::forward<Pred>(pred)]<class T>(T&& value) {
            return NulloptIfNot(pred, std::forward<T>(value));
        };
    }

    //! @hof{NulloptIfNot}
    template<auto Pred>
    constexpr auto H_NulloptIfNot() {
        return H_NulloptIfNot(ToValue<Pred>());
    }

#pragma endregion
}

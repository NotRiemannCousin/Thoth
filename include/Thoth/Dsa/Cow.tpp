#pragma once

namespace Thoth::Dsa {
    template<class RefT, class OwnT> requires std::constructible_from<OwnT, RefT>
    constexpr Cow<RefT, OwnT>::Cow(const Cow& other) {
        _value = other._value;
        AsOwned();
    }

    template<class RefT, class OwnT> requires std::constructible_from<OwnT, RefT>
    constexpr Cow<RefT, OwnT>& Cow<RefT, OwnT>::operator=(const Cow& other) {
        if (this == &other)
            return *this;

        _value = other._value;
        AsOwned();
        return *this;
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT>
    constexpr Cow<RefT, OwnT> Cow<RefT, OwnT>::FromRef(RefT ref) {
        Cow obj;
        obj._value = ref;

        return obj;
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT>
    constexpr Cow<RefT, OwnT>& Cow<RefT, OwnT>::SetRef(RefT ref) {
        _value = ref;
        return *this;
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT>
    constexpr bool Cow<RefT, OwnT>::IsRefType(const Cow& cow) {
        return std::holds_alternative<RefT>(cow._value);
    }


    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT>
    constexpr bool Cow<RefT, OwnT>::IsRef() const {
        return IsRefType(*this);
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT>
    constexpr Cow<RefT, OwnT> Cow<RefT, OwnT>::FromOwned(const OwnT& own) {
        Cow obj;
        obj._value = own;

        return obj;
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT>
    constexpr Cow<RefT, OwnT> Cow<RefT, OwnT>::FromOwned(OwnT &&own) {
        Cow obj;
        obj._value = std::move(own);

        return obj;
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT>
    constexpr Cow<RefT, OwnT>& Cow<RefT, OwnT>::SetOwned(const OwnT& own) {
        _value = own;
        return *this;
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT>
    constexpr Cow<RefT, OwnT>& Cow<RefT, OwnT>::SetOwned(OwnT&& own) {
        _value = std::move(own);
        return *this;
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT>
    constexpr OwnT& Cow<RefT, OwnT>::AsOwned() {
        if (std::holds_alternative<RefT>(_value))
            _value = OwnT(std::get<RefT>(_value));
        return std::get<OwnT>(_value);
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT>
    constexpr RefT Cow<RefT, OwnT>::AsRef() const {
        if (std::holds_alternative<RefT>(_value))
            return std::get<RefT>(_value);
        return RefT{ std::get<OwnT>(_value) };
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT>
    template<class Callable>
    constexpr decltype(auto) Cow<RefT, OwnT>::Visit(Callable&& callable) {
        return std::visit(std::forward<Callable>(callable), _value);
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT>
    template<class Callable>
    constexpr decltype(auto) Cow<RefT, OwnT>::Visit(Callable&& callable) const {
        return std::visit(std::forward<Callable>(callable), _value);
    }
}

#pragma once
#include <format>

namespace Thoth::Dsa {
    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr Cow<RefT, OwnT>::Cow(RefT ref) noexcept : m_value{ ref } { }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr Cow<RefT, OwnT>::Cow(const Cow& other) {
        m_value = other.m_value;
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr Cow<RefT, OwnT>& Cow<RefT, OwnT>::operator=(const Cow& other) {
        if (this != &other)
            m_value = other.m_value;
        return *this;
    }


#pragma region Ref

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr Cow<RefT, OwnT>::operator RefT() const {
        return AsRef();
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr Cow<RefT, OwnT> Cow<RefT, OwnT>::FromRef(RefT ref) {
        Cow obj;
        obj.m_value = ref;

        return obj;
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr Cow<RefT, OwnT>& Cow<RefT, OwnT>::SetRef(RefT ref) {
        m_value = ref;
        return *this;
    }

#pragma endregion


    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr bool Cow<RefT, OwnT>::IsRefType(const Cow& cow) {
        return std::holds_alternative<RefT>(cow.m_value);
    }


    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr bool Cow<RefT, OwnT>::IsRef() const {
        return IsRefType(*this);
    }


#pragma region Own

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr Cow<RefT, OwnT> Cow<RefT, OwnT>::FromOwned(const OwnT& own) {
        Cow obj;
        obj.m_value = own;

        return obj;
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr Cow<RefT, OwnT> Cow<RefT, OwnT>::FromOwned(OwnT &&own) {
        Cow obj;
        obj.m_value = std::move(own);

        return obj;
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr Cow<RefT, OwnT>& Cow<RefT, OwnT>::SetOwned(const OwnT& own) {
        m_value = own;
        return *this;
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr Cow<RefT, OwnT>& Cow<RefT, OwnT>::SetOwned(OwnT&& own) {
        m_value = std::move(own);
        return *this;
    }

#pragma endregion


#pragma region "As" converters

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr OwnT& Cow<RefT, OwnT>::AsOwned() {
        if (std::holds_alternative<RefT>(m_value))
            m_value = OwnT(std::get<RefT>(m_value));
        return std::get<OwnT>(m_value);
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr OwnT Cow<RefT, OwnT>::AsCopy() const {
        if (std::holds_alternative<RefT>(m_value))
            return OwnT{ std::get<RefT>(m_value) };
        return std::get<OwnT>(m_value);
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr RefT Cow<RefT, OwnT>::AsRef() const {
        if (std::holds_alternative<RefT>(m_value))
            return std::get<RefT>(m_value);
        return RefT{ std::get<OwnT>(m_value) };
    }

#pragma endregion


#pragma region Visit

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    template<class Callable>
    constexpr decltype(auto) Cow<RefT, OwnT>::Visit(Callable&& callable) {
        return std::visit(std::forward<Callable>(callable), m_value);
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    template<class Callable>
    constexpr decltype(auto) Cow<RefT, OwnT>::Visit(Callable&& callable) const {
        return std::visit(std::forward<Callable>(callable), m_value);
    }

#pragma endregion


#pragma region Ranges

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr auto Cow<RefT, OwnT>::begin() const
        requires requires(RefT r) { std::ranges::begin(r); } {
        return std::ranges::begin(AsRef());
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr auto Cow<RefT, OwnT>::end() const
        requires requires(RefT r) { std::ranges::end(r); } {
        return std::ranges::end(AsRef());
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr auto Cow<RefT, OwnT>::cbegin() const
        requires requires(RefT r) { std::ranges::cbegin(r); } {
        return std::ranges::cbegin(AsRef());
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr auto Cow<RefT, OwnT>::cend() const
        requires requires(RefT r) { std::ranges::cend(r); } {
        return std::ranges::cend(AsRef());
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr auto Cow<RefT, OwnT>::rbegin() const
        requires requires(RefT r) { std::ranges::rbegin(r); } {
        return std::ranges::rbegin(AsRef());
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr auto Cow<RefT, OwnT>::rend() const
        requires requires(RefT r) { std::ranges::rend(r); } {
        return std::ranges::rend(AsRef());
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr auto Cow<RefT, OwnT>::size() const
        requires requires(RefT r) { std::ranges::size(r); } {
        return std::ranges::size(AsRef());
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr auto Cow<RefT, OwnT>::ssize() const
        requires requires(RefT r) { std::ranges::ssize(r); } {
        return std::ranges::ssize(AsRef());
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr bool Cow<RefT, OwnT>::empty() const
        requires requires(RefT r) { std::ranges::empty(r); } {
        return std::ranges::empty(AsRef());
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr auto Cow<RefT, OwnT>::data() const
        requires requires(RefT r) { std::ranges::data(r); } {
        return std::ranges::data(AsRef());
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr auto Cow<RefT, OwnT>::front() const
        requires requires(RefT r) { r.front(); } {
        return AsRef().front();
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr auto Cow<RefT, OwnT>::back() const
        requires requires(RefT r) { r.back(); } {
        return AsRef().back();
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    template <typename Index>
    constexpr auto Cow<RefT, OwnT>::operator[](Index idx) const
        requires requires(RefT r, Index i) { r[i]; } {
        return AsRef()[idx];
    }

    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    template <typename U>
    constexpr bool Cow<RefT, OwnT>::contains(const U& val) const
        requires requires(RefT r, U v) { r.contains(v); } {
        return AsRef().contains(val);
    }

#pragma endregion


    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
    constexpr bool operator==(const Cow<RefT, OwnT>& left, const Cow<RefT, OwnT>& right) {
        return left.Visit([&]<typename T0>(const T0& l) {
            return right.Visit([&]<typename T1>(const T1& r) -> bool {
                using L = std::remove_cvref_t<T0>;
                using R = std::remove_cvref_t<T1>;

                if constexpr (std::same_as<L, R>)
                    return l == r;
                else if constexpr (std::same_as<L, RefT>)
                    return l == RefT{ r };
                else
                    return RefT{ l } == r;
            });
        });
    }
}

template<class RefT, class OwnT>
    requires requires(RefT ref){ { std::format("{}", ref) }; } && requires(OwnT own){ { std::format("{}", own) }; }
struct std::formatter<Thoth::Dsa::Cow<RefT, OwnT>>{
    static constexpr auto parse(auto& ctx) { return ctx.begin(); }

    auto format(const Thoth::Dsa::Cow<RefT, OwnT> &cow, auto& ctx) const {
        cow.Visit([&](auto val){ std::format_to(ctx.out(), "{}", val); });

        return ctx.out();
    }
};
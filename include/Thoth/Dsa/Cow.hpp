#pragma once
#include <concepts>
#include <variant>

namespace Thoth::Dsa {
    //! @brief Like Rust's `cow` (copy on write).
    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT>
    struct Cow {
        using ValueType = std::variant<RefT, OwnT>;

        using RefType = RefT;
        using OwnType = OwnT;

        constexpr Cow() = default;
        constexpr Cow(Cow&&) noexcept = default;
        constexpr Cow(const Cow& other);

        constexpr Cow& operator=(Cow&& other) noexcept = default;
        constexpr Cow& operator=(const Cow& other);

        constexpr bool operator==(const Cow&) const = default;

        constexpr static Cow FromRef(RefT ref);
        constexpr Cow& SetRef(RefT ref);


        [[nodiscard]] constexpr static bool IsRefType(const Cow& cow);
        [[nodiscard]] constexpr bool IsRef() const;


        constexpr static Cow FromOwned(const OwnT& own);
        constexpr static Cow FromOwned(OwnT&& own);
        constexpr Cow& SetOwned(const OwnT& own);
        constexpr Cow& SetOwned(OwnT&& own);

        constexpr OwnT& AsOwned();

        [[nodiscard]] constexpr RefT AsRef() const;

        template<class Callable>
        constexpr decltype(auto) Visit(Callable&& callable);

        template<class Callable>
        [[nodiscard]] constexpr decltype(auto) Visit(Callable&& callable) const;
    private:
        ValueType _value;
    };
}

#include <Thoth/Dsa/Cow.tpp>

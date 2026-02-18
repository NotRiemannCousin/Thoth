#pragma once
#include <concepts>
#include <variant>

namespace Thoth::Dsa {
    //! @brief Like Rust's `cow` (copy on write).
    //!
    //! The value can a RefT or a OwnT. RefT are lightweight objects designed to
    //! reference external buffers, while OwnT are objects that stores it own data
    //! so they can be heavy to create and copy.
    //! Editing the Cow triggers transform the RefT in OwnT.
    //! You need to be able to construct OwnT from RefT and RefT from OwnT.
    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT>
    struct Cow {
        using ValueType = std::variant<RefT, OwnT>;

        using RefType = RefT;
        using OwnType = OwnT;

        // NOLINTBEGIN(*)
        constexpr Cow() = default;
        constexpr Cow(RefT) noexcept;
        constexpr Cow(Cow&&) noexcept = default;
        constexpr Cow(const Cow& other);
        // NOLINTEND(*)

        constexpr Cow& operator=(Cow&& other) noexcept = default;
        constexpr Cow& operator=(const Cow& other);


        //! @brief Constructs a Cow from a RefT.
        static constexpr Cow FromRef(RefT ref);
        //! @brief Sets the value to a new RefT.
        constexpr Cow& SetRef(RefT ref);


        //! @brief Check if the value is a RefT or OwnT.
        [[nodiscard]] static constexpr bool IsRefType(const Cow& cow);
        //! @brief Check if the value is a RefT or OwnT.
        [[nodiscard]] constexpr bool IsRef() const;


        //! @brief Create a OwnT value from another OwnT.
        static constexpr Cow FromOwned(const OwnT& own);
        //! @brief Create a OwnT value from another OwnT.
        static constexpr Cow FromOwned(OwnT&& own);
        //! @brief Set value to a new OwnT from another OwnT.
        constexpr Cow& SetOwned(const OwnT& own);
        //! @brief Set value to a new OwnT from another OwnT.
        constexpr Cow& SetOwned(OwnT&& own);

        //! @brief Converts the RefT type to a OwnT.
        constexpr OwnT& AsOwned();
        //! @brief Returns a copy of the value as OwnT without modifying it.
        [[nodiscard]] constexpr OwnT AsCopy() const;

        //! @brief Returns a reference of the value as RefT without modifying it.
        [[nodiscard]] constexpr RefT AsRef() const;

        //! @brief convenient call to std::visit() on _value.
        template<class Callable>
        constexpr decltype(auto) Visit(Callable&& callable);

        //! @brief convenient call to std::visit() on _value.
        template<class Callable>
        [[nodiscard]] constexpr decltype(auto) Visit(Callable&& callable) const;
    private:
        ValueType _value;
    };


    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT>
    constexpr bool operator==(const Cow<RefT, OwnT>& left, const Cow<RefT, OwnT>& right);
}

#include <Thoth/Dsa/Cow.tpp>

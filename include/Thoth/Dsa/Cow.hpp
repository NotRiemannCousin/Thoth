#pragma once
#include <concepts>
#include <variant>

namespace Thoth::Dsa {
    //! @brief A Copy-On-Write (COW) smart wrapper, inspired by Rust's `std::borrow::Cow`.
    //!
    //! Encapsulates either a borrowed reference (@c RefT) or an owned value (@c OwnT). It optimizes memory and
    //! performance by favoring lightweight references, only allocating/copying to an owned type when mutation
    //! is explicitly required via @c AsOwned().
    //!
    //! @note Requires bidirectional convertibility, @c OwnT from @c RefT, and @c RefT from <tt>const OwnT&</tt>.
    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT> && std::constructible_from<RefT, const OwnT&>
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


        [[nodiscard]] constexpr operator RefT() const;

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



        //! @brief convenient call to std::visit() on m_value.
        template<class Callable>
        constexpr decltype(auto) Visit(Callable&& callable);

        //! @brief convenient call to std::visit() on m_value.
        template<class Callable>
        [[nodiscard]] constexpr decltype(auto) Visit(Callable&& callable) const;


#pragma region Ranges

        [[nodiscard]] constexpr auto begin()  const requires requires(RefT r) { std::ranges::begin(r);  };
        [[nodiscard]] constexpr auto end()    const requires requires(RefT r) { std::ranges::end(r);    };
        [[nodiscard]] constexpr auto cbegin() const requires requires(RefT r) { std::ranges::cbegin(r); };
        [[nodiscard]] constexpr auto cend()   const requires requires(RefT r) { std::ranges::cend(r);   };
        [[nodiscard]] constexpr auto rbegin() const requires requires(RefT r) { std::ranges::rbegin(r); };
        [[nodiscard]] constexpr auto rend()   const requires requires(RefT r) { std::ranges::rend(r);   };

        [[nodiscard]] constexpr auto size()  const requires requires(RefT r) { std::ranges::size(r);  };
        [[nodiscard]] constexpr auto ssize() const requires requires(RefT r) { std::ranges::ssize(r); };
        [[nodiscard]] constexpr bool empty() const requires requires(RefT r) { std::ranges::empty(r); };

        [[nodiscard]] constexpr auto data()  const requires requires(RefT r) { std::ranges::data(r); };
        [[nodiscard]] constexpr auto front() const requires requires(RefT r) { r.front(); };
        [[nodiscard]] constexpr auto back()  const requires requires(RefT r) { r.back();  };

        template <typename Index>
        [[nodiscard]] constexpr auto operator[](Index idx) const requires requires(RefT r, Index i) { r[i]; };

        template <typename U>
        [[nodiscard]] constexpr bool contains(const U& val) const requires requires(RefT r, U v) { r.contains(v); };

#pragma endregion
    private:
        ValueType m_value;
    };


    template<class RefT, class OwnT>
        requires std::constructible_from<OwnT, RefT>
    constexpr bool operator==(const Cow<RefT, OwnT>& left, const Cow<RefT, OwnT>& right);
}

#include <Thoth/Dsa/Cow.tpp>

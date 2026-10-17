#pragma once
#include <cassert>
#include <cstddef>
#include <iterator>
#include <ranges>
#include <type_traits>
#include <utility>
#include <variant>

#include <Thoth/NJson/Definitions.hpp>
#include <Thoth/NJson/JsonObject.hpp>
#include <Thoth/NJson/Keys.hpp>

namespace Thoth::NJson {

#pragma region Concepts

    //! @cond INTERNAL
    template<bool IsConst>
    concept ArrayChildren = requires { typename std::conditional_t<IsConst, Array::const_iterator, Array::iterator>; };
    //! @endcond

#pragma endregion

#pragma region JsonIterator

    //! @brief Bidirectional iterator over the children of a @ref Json.
    //!
    //! For @c Array values yields each element; for @c Object values yields each
    //! child value (keys are skipped — see @ref Json::Items for paired iteration).
    //! Every other type (@ref Null, @ref String, @ref Number, @ref Bool) has zero
    //! children, so every @ref Json is uniformly iterable.
    //!
    //! @tparam IsConst     When true children are read-only.
    //! @tparam IsItems     When true @c operator* yields `std::pair<KeyView, Json&>`
    //!                     (array index, or object entry key) instead of just @ref Json&.
    template<bool IsConst, bool IsItems = false>
    struct JsonIterator {
        template<bool, bool>
        friend struct JsonIterator;

        using JsonPtr = std::conditional_t<IsConst, const Json*, Json*>;
        using JsonRef = std::conditional_t<IsConst, const Json&, Json&>;

        using reference  = std::conditional_t<IsItems, std::pair<KeyView, JsonRef>, JsonRef>;
        using value_type = std::conditional_t<IsItems, std::pair<KeyView, Json   >, Json   >;

        using difference_type   = std::ptrdiff_t;
        using iterator_concept  = std::bidirectional_iterator_tag;
        using iterator_category = iterator_concept;

        using pointer = std::conditional_t<IsItems, void, JsonPtr>;

        // -- Internal states -------------------------------------------------

        struct Scalar  {};  // empty sequence (null, string, number, bool)
        struct ObjectState {
            std::conditional_t<IsConst, JsonObject::CIterType, JsonObject::IterType> iter;
        };
        struct ArrayState {
            std::conditional_t<IsConst, Array::const_iterator, Array::iterator> iter;
            std::ptrdiff_t index;
        };

        using State = std::variant<Scalar, ObjectState, ArrayState>;

        constexpr JsonIterator() = default;

        //! @brief Creates an iterator over the children of @p parent.
        //! @param parent Json whose children will be walked. Scalars yield an empty sequence.
        //! @param atEnd  Builds the one-past-the-last iterator when true.
        explicit JsonIterator(JsonPtr parent, bool atEnd = false);

        //! @brief Allows a mutable iterator to decay into a const one.
        template<bool WasConst>
            requires IsConst && (!WasConst)
        JsonIterator(const JsonIterator<WasConst, IsItems>& other);

        [[nodiscard]] reference operator*() const;

        [[nodiscard]] JsonPtr operator->() const requires (!IsItems);

        JsonIterator& operator++();
        [[nodiscard]] JsonIterator operator++(int);

        JsonIterator& operator--();
        [[nodiscard]] JsonIterator operator--(int);

        [[nodiscard]] bool operator==(const JsonIterator&) const = default;

    private:
        State m_state{};
    };

    //! @brief Iterator yielding `std::pair<KeyView, Json&>`.
    template<bool IsConst>
    using JsonItemsIterator = JsonIterator<IsConst, true>;

#pragma endregion

#pragma region JsonItemsView

    //! @brief A @c std::ranges view of `std::pair<KeyView, Json&>` over the children of a Json.
    //!
    //! Pairs each child with its array index (for @c Array values) or entry key
    //! (for @c Object values). Scalars and null yield an empty view. String keys are
    //! @c string_views into the parent, valid while the parent keeps its entries.
    //!
    //! Models @c std::ranges::view_interface (and therefore @c forward_range,
    //! @c common_range and @c sized_range). Not a proxy-iterator design: each
    //! @c operator* builds a fresh pair, so only single-pass traversal is guaranteed
    //! for the const view; the mutable view is bidirectional.
    //!
    //! @note References its parent; does NOT extend the parent's lifetime.
    template<bool IsConst>
    class JsonItemsView : public std::ranges::view_interface<JsonItemsView<IsConst>> {
        template<bool>
        friend class JsonItemsView;

    public:
        using JsonPtr    = std::conditional_t<IsConst, const Json*, Json*>;
        using iterator   = JsonItemsIterator<IsConst>;
        using const_iterator = JsonItemsIterator<true>;

        constexpr JsonItemsView() = default;

        //! @brief Views (key, child) pairs of @p parent. Scalars/null → empty view.
        constexpr explicit JsonItemsView(JsonPtr parent) noexcept : m_parent{ parent } {}

        //! @brief Mutable → const decay.
        template<bool WasConst>
            requires IsConst && (!WasConst)
        JsonItemsView(const JsonItemsView<WasConst>& other) noexcept : m_parent{ other.m_parent } {}

        [[nodiscard]] iterator begin() const;
        [[nodiscard]] iterator end() const;

        [[nodiscard]] bool empty() const;
        [[nodiscard]] std::size_t size() const;

    private:
        JsonPtr m_parent{};
    };

#pragma endregion

}  // namespace Thoth::NJson

template<bool IsConst>
inline constexpr bool std::ranges::enable_borrowed_range<Thoth::NJson::JsonItemsView<IsConst>> = true;

#include <Thoth/NJson/Range.tpp>

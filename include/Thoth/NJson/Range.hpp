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

    //! @brief Bidirectional iterator over the children of a @ref Json. It is designed to work only with @c Object and
    //! @c Array.
    //!
    //! It @c IsItems is true, it yields all `(KeyView key, Json value)` pairs, with `KeyView` being the array indices
    //! or object key strings. Otherwise, it yields only the @ref Json values.
    //!
    //! @note Despites being useless, for API simplicity this type always works with `(KeyView key, StateIter it)` and
    //! select IsItems will only change dereferencialization.
    //!
    //! @see Json::Items.
    template<bool IsConst, bool IsItems = true>
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

        using ObjectIter = std::conditional_t<IsConst, JsonObject::CIterType, JsonObject::IterType>;
        using ArrayIter  = std::conditional_t<IsConst, Array::const_iterator, Array::iterator>;

        using ArrayProxyIter = std::pair<std::size_t, ArrayIter>;

        using State = std::variant<ObjectIter, ArrayProxyIter>;

        constexpr JsonIterator() = default;

        constexpr explicit JsonIterator(ObjectIter it);
        constexpr explicit JsonIterator(ArrayIter it, std::size_t index);

        //! @brief Allows a mutable iterator to decay into a const one.
        template<bool WasConst>
            requires IsConst && (!WasConst)
        JsonIterator(const JsonIterator<WasConst, IsItems>& other);

        [[nodiscard]] reference operator*() const;

        JsonIterator& operator++();
        [[nodiscard]] JsonIterator operator++(int);

        JsonIterator& operator--();
        [[nodiscard]] JsonIterator operator--(int);

        [[nodiscard]] bool operator==(const JsonIterator&) const = default;

    private:
        State m_state{};
    };


#pragma endregion

#pragma region JsonItemsView

    //! @brief A @c std::ranges view yielding `(KeyView, Json&)` pairs over the children of a @ref Json.
    //!
    //! Pairs each child with its array index or object key. Scalars and null yield an empty view. It models
    //! @c std::ranges::view_interface. String keys are @c string_view objects tied to the parent's lifetime.
    //!
    //! @note This view holds a non-owning reference to its parent. The parent must outlive the view.
    template<bool IsConst>
    class JsonItemsView : public std::ranges::view_interface<JsonItemsView<IsConst>> {
        template<bool>
        friend class JsonItemsView;

    public:
        using iterator       = JsonIterator<IsConst>;
        using const_iterator = JsonIterator<true>;

        using JsonPtr = typename iterator::JsonPtr;

        constexpr JsonItemsView() = default;

        //! @brief Views (key, child) pairs of @p parent. Scalars/null → empty view.
        constexpr explicit JsonItemsView(JsonPtr parent) noexcept;

        //! @brief Mutable → const decay.
        template<bool WasConst>
            requires IsConst && (!WasConst)
        JsonItemsView(const JsonItemsView<WasConst>& other) noexcept;

        template<class=void> [[nodiscard]] iterator begin() noexcept requires (!IsConst);
        template<class=void> [[nodiscard]] iterator end() noexcept requires (!IsConst);
        [[nodiscard]] const_iterator begin() const noexcept;
        [[nodiscard]] const_iterator end() const noexcept;

        [[nodiscard]] bool empty() const;
        [[nodiscard]] std::size_t size() const;

    private:
        JsonPtr m_parent{};
    };

#pragma endregion

}

#include <Thoth/NJson/Range.tpp>
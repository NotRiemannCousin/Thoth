#pragma once
#include <variant>
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <expected>
#include <algorithm>
#include <functional>
#include <concepts>
#include <span>

#include <Thoth/NJson/Definitions.hpp>
#include <Thoth/NJson/Range.hpp>
#include <Thoth/NJson/Keys.hpp>
#include <Thoth/ThothError.hpp>

namespace Thoth::NJson {
    struct Json {
        using Value = std::variant<Null, String, Number, Bool, Object, Array>;
        using PredicatePointer = bool(*)(const Json&);

        // NOLINTBEGIN(*)
        Json();
        Json(const JsonObject& child);
        Json(JsonObject&& child);

        Json(const Array& child);
        Json(Array&& child);

        Json(Value&& newValue) noexcept;
        Json(const Value& newValue);
        Json(Json&& other) noexcept;
        Json(const Json& other);

        Json(bool other);
        template<class T>
            requires std::floating_point<T> || std::integral<T> && (!std::same_as<T, bool>)
        Json(T other);
        template<class T>
            requires std::constructible_from<std::string, T>
        Json(T&& other);

        Json& operator=(const JsonObject& other);
        Json& operator=(JsonObject&& other);

        Json& operator=(const Array& child);
        Json& operator=(Array&& child);

        Json& operator=(Value&& newValue) noexcept;
        Json& operator=(const Value& newValue);
        Json& operator=(Json&& other) noexcept;
        Json& operator=(const Json& other);

        Json& operator=(bool other);
        template<class T>
            requires std::floating_point<T> || std::integral<T> && (!std::same_as<T, bool>)
        Json& operator=(T other);
        template<class T>
            requires std::constructible_from<std::string, T>
        Json& operator=(T&& other);


        operator Value&();
        [[nodiscard]] operator const Value&() const;
        // NOLINTEND(*)

        template<class T>
        static bool IsOfType(const Json& val);
        template<class T>
        [[nodiscard]] bool IsOf() const;

        template<class T>
        static T& AsType(Json& val);

        template<class T, class Self>
        [[nodiscard]] decltype(auto) As(this Self&& self);

        template<class T>
        [[nodiscard]] T AsCpy() const&;
        template<class T>
        [[nodiscard]] T& AsMut() &;
        template<class T>
        [[nodiscard]] T&& AsMov() &&;
        template<class T>
        [[nodiscard]] const T& AsRef() const&;

        template<class T>
        [[nodiscard]] std::optional<T*> Ensure() &;
        template<class T>
        [[nodiscard]] std::optional<T> Ensure() &&;
        template<class T>
        [[nodiscard]] std::optional<const T*> Ensure() const&;

        template<class T>
        [[nodiscard]] std::optional<T> EnsureCpy() const&;
        template<class T>
        [[nodiscard]] std::optional<T*> EnsureMut() &;
        template<class T>
        [[nodiscard]] std::optional<T> EnsureMov() &&;
        template<class T>
        [[nodiscard]] std::optional<const T*> EnsureRef() const&;

        template<class T>
        [[nodiscard]] std::expected<T*, ThothError> EnsureOrError() &;
        template<class T>
        [[nodiscard]] std::expected<T, ThothError> EnsureOrError() &&;
        template<class T>
        [[nodiscard]] std::expected<const T*, ThothError> EnsureOrError() const&;

        template<class T>
        [[nodiscard]] std::expected<T, ThothError> EnsureCpyOrError() const&;
        template<class T>
        [[nodiscard]] std::expected<T*, ThothError> EnsureMutOrError() &;
        template<class T>
        [[nodiscard]] std::expected<T, ThothError> EnsureMovOrError() &&;
        template<class T>
        [[nodiscard]] std::expected<const T*, ThothError> EnsureRefOrError() const&;

        [[nodiscard]] bool operator==(const Json& other) const;

        //! @brief Tries to parse the Json from a string.
        //! @details Requires only one parameter so it's convenient to monads. Calls ParseText with default parameters.
        //! @param input the text to parse.
        //! @return A Json if the parse success, std::nullopt otherwise.
        static std::expected<Json, ThothError> Parse(std::string_view input);


        //! @copybrief Parse
        //! @param input the text to parse.
        //! @param copyData copy the input to an internal buffer if true, keeps a reference otherwise.
        //! @param checkFinal ensure that there is only space chars after the end of the json.
        //! @return A Json if the parse success, std::nullopt otherwise.
        static std::expected<Json, ThothError> ParseText(std::string_view input, bool copyData = true, bool checkFinal = true);


#pragma region Get Functions
        //! @{
        //! @name Get Functions
        //! Will get the direct child of this element with a given key/index.

        //! @brief Return a ref of the element with this index/key if this is an Object or Array, std::nullopt otherwise.
        OptRefValWrapper Get(KeyView key);
        //! @copybrief Get
        [[nodiscard]] OptCRefValWrapper Get(KeyView key) const;
        //! @brief Return a copy of the element with this index/key if this is an Object or Array, std::nullopt otherwise.
        [[nodiscard]] OptValWrapper GetCopy(KeyView key) const;
        //! @brief Return (with move) the element with this index/key if this is an Object or Array, std::nullopt otherwise.
        OptValWrapper GetAndMove(KeyView key) &&;


        //! @brief Return a copy of the element with this index/key if this is an Object or Array, null otherwise.
        [[nodiscard]] ValWrapper GetCopyOrNull(KeyView key) const;
        //! @brief Move the element with this index/key if this is an Object or Array, null otherwise.
        ValWrapper GetAndMoveOrNull(KeyView key) &&;


        //! @brief Return a ref of the element with this index/key if this is an Object or Array, ThothError otherwise.
        ExpRefValWrapper GetOrError(KeyView key);
        //! @copybrief GetOrError
        [[nodiscard]] ExpCRefValWrapper GetOrError(KeyView key) const;
        //! @brief Return a copy of the element with this index/key if this is an Object or Array, ThothError otherwise.
        [[nodiscard]] ExpValWrapper GetCopyOrError(KeyView key) const;
        //! @brief Move the element with this index/key if this is an Object or Array. Return ThothError otherwise.
        ExpValWrapper GetAndMoveOrError(KeyView key) &&;

        //! @}

#pragma endregion

#pragma region Find Functions
        //! @{
        //! @name Find Functions
        //! Will find a nested child in the tree. Something like `["data"][-1]["contents"][0]["name"]`.

        //! @brief Same as successive calls to Get, std::nullopt at the first fail.
        OptRefValWrapper Find(KeysView keys);
        //! @copybrief Find
        [[nodiscard]] OptCRefValWrapper Find(KeysView keys) const;
        //! @brief Same as successive calls to GetCopy, std::nullopt at the first fail.
        [[nodiscard]] OptValWrapper FindCopy(KeysView keys) const;
        //! @brief Same as successive calls to GetAndMove, std::nullopt at the first fail.
        OptValWrapper FindAndMove(KeysView keys) &&;


        //! @brief Same as successive calls to GetCopy. Return null at the first fail.
        [[nodiscard]] ValWrapper FindCopyOrNull(KeysView keys) const;
        //! @brief Same as successive calls to GetAndMove. Return null at the first fail.
        ValWrapper FindAndMoveOrNull(KeysView keys) &&;


        //! @brief Same as successive calls to Get. Return ThothError at the first fail.
        ExpRefValWrapper FindOrError(KeysView keys);
        //! @copybrief FindOrError
        [[nodiscard]] ExpCRefValWrapper FindOrError(KeysView keys) const;
        //! @brief Same as successive calls to GetCopy. Return ThothError at the first fail.
        [[nodiscard]] ExpValWrapper FindCopyOrError(KeysView keys) const;
        //! @brief Same as successive calls to Find. Return ThothError at the first fail.
        ExpValWrapper FindAndMoveOrError(KeysView keys) &&;

        //! @}
#pragma endregion

#pragma region Search Functions
        //! @{
        //! @name Search Functions
        //! Will pick the fist element that satisfies the given predicate.

        //! @brief Will search the childs for the first element that matches the predicate and return it, or
        //! std::nullopt if no matches.
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        OptRefValWrapper Search(Pred&& pred);
        //! @copybrief Search
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        [[nodiscard]] OptCRefValWrapper Search(Pred&& pred) const;
        //! @brief Will search the childs for the first element that matches the predicate and clone it, or
        //! std::nullopt if no matches.
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        [[nodiscard]] OptValWrapper SearchCopy(Pred&& pred) const;
        //! @brief Will search the childs for the first element that matches the predicate and move it, or std::nullopt
        //! if no matches.
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        OptValWrapper SearchAndMove(Pred&& pred) &&;

        //! @brief Will search the childs for the first element that matches the predicate and clone it, or ThothError
        //! if no matches.
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        [[nodiscard]] ValWrapper SearchCopyOrNull(Pred&& pred) const;
        //! @brief Will search the childs for the first element that matches the predicate and move it, or ThothError
        //! if no matches.
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        ValWrapper SearchAndMoveOrNull(Pred&& pred);


        //! @brief Will search the childs for the first element that matches the predicate and return it, or
        //! std::nullopt if no matches.
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        ExpRefValWrapper SearchOrError(Pred&& pred);
        //! @copybrief SearchOrError
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        [[nodiscard]] ExpCRefValWrapper SearchOrError(Pred&& pred) const;
        //! @brief Will search the childs for the first element that matches the predicate and clone it, or std::nullopt
        //! if no matches.
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        [[nodiscard]] ExpValWrapper SearchCopyOrError(Pred&& pred) const;
        //! @brief Will search the childs for the first element that matches the predicate and move it, or std::nullopt
        //! if no matches.
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        ExpValWrapper SearchAndMoveOrError(Pred&& pred) &&;

        //! @}
#pragma endregion


#pragma region Range Interface
        //! @{
        //! @name Range Interface
        //! Every Json models the STL range concepts over its direct children:
        //! - @c Array values iterate their elements;
        //! - @c Object values iterate their values, in storage order (keys are skipped);
        //! - every other type (null, string, number, bool) is an empty sequence.
        //!
        //! @c begin()/@c end() return the same iterator type and the iterators are bidirectional,
        //! so a Json always models @c std::ranges::forward_range, @c std::ranges::bidirectional_range,
        //! @c std::ranges::common_range and @c std::ranges::sized_range — mutable and const alike.
        //! When the keys are needed, @see Items() pairs each child with its @ref KeyView; for a
        //! guaranteed index, compose with @c std::views::enumerate.

        using iterator       = JsonIterator<false, false>;
        using const_iterator = JsonIterator<true , false>;
        using value_type     = Json;

        //! @brief Iterator to the first child. Scalar values yield @c begin() == @c end().
        iterator begin();
        //! @brief Iterator to one-past-the-last child.
        iterator end();
        //! @copydoc begin
        [[nodiscard]] const_iterator begin() const;
        //! @copydoc end
        [[nodiscard]] const_iterator end() const;
        //! @brief Const iterator to the first child, even when this Json is mutable.
        [[nodiscard]] const_iterator cbegin() const;
        //! @brief Const iterator to one-past-the-last child, even when this Json is mutable.
        [[nodiscard]] const_iterator cend() const;

        //! @brief True when this value has no children (scalars are always empty).
        [[nodiscard]] bool Empty() const;
        //! @brief STL spelling of Empty(), kept so @c std::ranges::empty works on a Json.
        [[nodiscard]] bool empty() const;

        //! @brief Number of children: elements of an Array, entries of an Object, 0 for scalars.
        [[nodiscard]] size_t Size() const;
        //! @brief STL spelling of Size(), kept so @c std::ranges::size works on a Json.
        [[nodiscard]] size_t size() const;

        //! @brief A @c std::ranges view of @c std::pair<KeyView, Json&> pairing each child with its
        //!        key: the array index for Array values, the entry key for Object values.
        //! @details Useful when the keys are needed, e.g.
        //!          @c for (const auto& [key, child] : json.Items()), or composing with
        //!          @c std::views::keys / @c std::views::values.
        //! @note The view references this Json, it does not extend its lifetime.
        [[nodiscard]] JsonItemsView<false> Items();
        //! @copydoc Items
        [[nodiscard]] JsonItemsView<true> Items() const;

        //! @}
#pragma endregion


#pragma region Visit

        //! @brief convenient call to std::visit() on m_value.
        template<class Callable>
        constexpr decltype(auto) Visit(Callable&& callable);

        //! @brief convenient call to std::visit() on m_value.
        template<class Callable>
        [[nodiscard]] constexpr decltype(auto) Visit(Callable&& callable) const;

#pragma endregion

    private:
        Value m_value;
    };

    inline static constexpr Null NullV{};
    inline static Json NullJ{};
}


#include <Thoth/NJson/Json.tpp>

static_assert(std::ranges::range<Thoth::NJson::Json               > && "Json is not a range"         );
static_assert(std::ranges::range<Thoth::NJson::JsonObject         > && "JsonObject is not a range"   );
static_assert(std::ranges::range<Thoth::NJson::JsonItemsView<true>> && "JsonItemsView is not a range");
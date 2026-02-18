#pragma once
#include <variant>
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <expected>
#include <concepts>
#include <span>

#include <Thoth/NJson/Definitions.hpp>
#include <Thoth/Http/RequestError.hpp>

namespace Thoth::NJson {

    struct Json{
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
        template<class T>
        [[nodiscard]] T& As();
        template<class T>
        [[nodiscard]] const T& As() const;

        template<class T>
        [[nodiscard]] T& AsMut();
        template<class T>
        [[nodiscard]] T AsMov() &&;
        template<class T>
        [[nodiscard]] const T& AsRef() const;

        template<class T>
        std::optional<T*> Ensure();
        template<class T>
        std::optional<T*> Ensure() const;

        template<class T>
        std::optional<T*> EnsureMut();
        template<class T>
        std::optional<T> EnsureMov() &&;
        template<class T>
        std::optional<const T*> EnsureRef() const;

        template<class T>
        std::expected<T*, Http::RequestError> EnsureOrError();
        template<class T>
        std::expected<T*, Http::RequestError> EnsureOrError() const;

        template<class T>
        std::expected<T*, Http::RequestError> EnsureMutOrError();
        template<class T>
        std::expected<T, Http::RequestError> EnsureMovOrError() &&;
        template<class T>
        std::expected<const T*, Http::RequestError> EnsureRefOrError() const;

        [[nodiscard]] bool operator==(const Json& other) const;

        //! @brief Tries to parse the Json from a string.
        //! @details Requires only one parameter so it's convenient to monads. Calls ParseText with default parameters.
        //! @param input the text to parse.
        //! @return A Json if the parse success, std::nullopt otherwise.
        static std::expected<Json, Http::RequestError> Parse(std::string_view input);


        //! @copybrief Parse
        //! @param input the text to parse.
        //! @param copyData copy the input to an internal buffer if true, keeps a reference otherwise.
        //! @param checkFinal ensure that there is only space chars after the end of the json.
        //! @return A Json if the parse success, std::nullopt otherwise.
        static std::expected<Json, Http::RequestError> ParseText(std::string_view input, bool copyData = true, bool checkFinal = true);


#pragma region Get Functions
        //! @{
        //! @name Get Functions
        //! Will get the direct child of this element with a given key/index.

        //! @brief Return a ref of the element with this index/key if this is an Object or Array, std::nullopt otherwise.
        OptRefValWrapper Get(Key key);
        //! @copybrief Get
        [[nodiscard]] OptCRefValWrapper Get(Key key) const;
        //! @brief Return a copy of the element with this index/key if this is an Object or Array, std::nullopt otherwise.
        [[nodiscard]] OptValWrapper GetCopy(Key key) const;
        //! @brief Return (with move) the element with this index/key if this is an Object or Array, std::nullopt otherwise.
        OptValWrapper GetAndMove(Key key) &&;


        //! @brief Return a copy of the element with this index/key if this is an Object or Array, null otherwise.
        [[nodiscard]] ValWrapper GetCopyOrNull(Key key) const;
        //! @brief Move the element with this index/key if this is an Object or Array, null otherwise.
        ValWrapper GetAndMoveOrNull(Key key) &&;


        //! @brief Return a ref of the element with this index/key if this is an Object or Array, RequestError otherwise.
        ExpRefValWrapper GetOrError(Key key);
        //! @copybrief GetOrError
        [[nodiscard]] ExpCRefValWrapper GetOrError(Key key) const;
        //! @brief Return a copy of the element with this index/key if this is an Object or Array, RequestError otherwise.
        [[nodiscard]] ExpValWrapper GetCopyOrError(Key key) const;
        //! @brief Move the element with this index/key if this is an Object or Array. Return RequestError otherwise.
        ExpValWrapper GetAndMoveOrError(Key key) &&;

        //! @}

#pragma endregion

#pragma region Find Functions
        //! @{
        //! @name Find Functions
        //! Will find a nested child in the tree. Something like `["data"][-1]["contents"][0]["name"]`.

        //! @brief Same as successive calls to Get, std::nullopt at the first fail.
        OptRefValWrapper Find(Keys keys);
        //! @copybrief Find
        [[nodiscard]] OptCRefValWrapper Find(Keys keys) const;
        //! @brief Same as successive calls to GetCopy, std::nullopt at the first fail.
        [[nodiscard]] OptValWrapper FindCopy(Keys keys) const;
        //! @brief Same as successive calls to GetAndMove, std::nullopt at the first fail.
        OptValWrapper FindAndMove(Keys keys) &&;


        //! @brief Same as successive calls to GetCopy. Return null at the first fail.
        [[nodiscard]] ValWrapper FindCopyOrNull(Keys keys) const;
        //! @brief Same as successive calls to GetAndMove. Return null at the first fail.
        ValWrapper FindAndMoveOrNull(Keys keys) &&;


        //! @brief Same as successive calls to Get. Return RequestError at the first fail.
        ExpRefValWrapper FindOrError(Keys keys);
        //! @copybrief FindOrError
        [[nodiscard]] ExpCRefValWrapper FindOrError(Keys keys) const;
        //! @brief Same as successive calls to GetCopy. Return RequestError at the first fail.
        [[nodiscard]] ExpValWrapper FindCopyOrError(Keys keys) const;
        //! @brief Same as successive calls to Find. Return RequestError at the first fail.
        ExpValWrapper FindAndMoveOrError(Keys keys) &&;

        //! @}
#pragma endregion

#pragma region Search Functions
        //! @{
        //! @name Search Functions
        //! Will pick the fist element that satisfies the given predicate.

        //! @brief Will search the childs for the first element that matches the predicate and return it, or std::nullopt if no matches.
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        OptRefValWrapper Search(Pred&& pred);
        //! @copybrief Search
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        [[nodiscard]] OptCRefValWrapper Search(Pred&& pred) const;
        //! @brief Will search the childs for the first element that matches the predicate and clone it, or std::nullopt if no matches.
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        [[nodiscard]] OptValWrapper SearchCopy(Pred&& pred) const;
        //! @brief Will search the childs for the first element that matches the predicate and move it, or std::nullopt if no matches.
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        OptValWrapper SearchAndMove(Pred&& pred) &&;

        //! @brief Will search the childs for the first element that matches the predicate and clone it, or RequestError if no matches.
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        [[nodiscard]] ValWrapper SearchCopyOrNull(Pred&& pred) const;
        //! @brief Will search the childs for the first element that matches the predicate and move it, or RequestError if no matches.
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        ValWrapper SearchAndMoveOrNull(Pred&& pred);


        //! @brief Will search the childs for the first element that matches the predicate and return it, or std::nullopt if no matches.
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        ExpRefValWrapper SearchOrError(Pred&& pred);
        //! @copybrief SearchOrError
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        [[nodiscard]] ExpCRefValWrapper SearchOrError(Pred&& pred) const;
        //! @brief Will search the childs for the first element that matches the predicate and clone it, or std::nullopt if no matches.
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        [[nodiscard]] ExpValWrapper SearchCopyOrError(Pred&& pred) const;
        //! @brief Will search the childs for the first element that matches the predicate and move it, or std::nullopt if no matches.
        template<class Pred = PredicatePointer> requires std::predicate<Pred, Json>
        ExpValWrapper SearchAndMoveOrError(Pred&& pred) &&;

        //! @}
#pragma endregion


        //! @brief convenient call to std::visit() on _value.
        template<class Callable>
        constexpr decltype(auto) Visit(Callable&& callable);

        //! @brief convenient call to std::visit() on _value.
        template<class Callable>
        [[nodiscard]] constexpr decltype(auto) Visit(Callable&& callable) const;
    private:
        Value _value;
    };

    inline static constexpr Null NullV{};
    inline static Json NullJ{};
}


#include <Thoth/NJson/Json.tpp>

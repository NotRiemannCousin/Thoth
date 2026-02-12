#pragma once
#include <variant>
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <concepts>
#include <span>

#include <Thoth/Dsa/Cow.hpp>
#include <Thoth/NJson/StringRef.hpp>

namespace Thoth::NJson {
    struct JsonObject;
    struct Json;


    using Null   = std::monostate;                   // null
    using String = Dsa::Cow<StringRef, std::string>; // string
    using Number = long double;                      // number
    using Bool   = bool;                             // bool
    using Object = std::unique_ptr<JsonObject>;      // {Object}
    using Array  = std::vector<Json>;                // [Array]

    namespace Details {
        struct BufferInfo {
            std::string_view bufferView;
            std::shared_ptr<std::string> buffer;
        };

        static bool ReadString(std::string_view& input, auto& val, const BufferInfo& info);
        static bool ReadNumber(std::string_view& input, auto& val);
        static bool ReadObject(std::string_view& input, auto& val, const BufferInfo& info);
        static bool ReadBool  (std::string_view& input, auto& val);
        static bool ReadNull  (std::string_view& input, auto& val);
        static bool ReadArray (std::string_view& input, auto& val, const BufferInfo& info);
    }


#pragma region Wappers for std::optional and std::expected
    using RefValWrapper = Json*;
    using CRefValWrapper = const Json*;

    using OptRefValWrapper = std::optional<RefValWrapper>;
    using OptCRefValWrapper = std::optional<CRefValWrapper>;

    using RefValWrapperOrNull = RefValWrapper;
    using CRefValWrapperOrNull = CRefValWrapper;

    using ValWrapper = Json;
    using CValWrapper = const Json;

    using OptValWrapper = std::optional<ValWrapper>;
    using OptCValWrapper = std::optional<CValWrapper>;

    using ValWrapperOrNull = ValWrapper;
    using CValWrapperOrNull = CValWrapper;
#pragma endregion



    using JsonObjKey    = std::string;
    using JsonObjKeyRef = std::string_view;

    using Key  = std::variant<int, JsonObjKey>;
    using Keys = std::span<const Key>;


    template<class ...T>
    requires ((std::unsigned_integral<T> || std::convertible_to<T, std::string_view>) &&...)
    auto MakeKeys(const T&... keys) { return std::array<Key, sizeof...(T)>{ (keys, ...) }; }

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

        [[nodiscard]] bool operator==(const Json& other) const;

        //! @brief Tries to parse the Json from a string.
        //! @param input the text to parse.
        static std::optional<Json> Parse(std::string_view input);


        //! @brief Tries to parse the Json from a string.
        //! @param input the text to parse.
        //! @param copyData copy the input to an internal buffer if true, keeps a reference otherwise.
        //! @param checkFinal ensure that there is only space chars after the end of the json.
        //! @return A Json if the parse success, std::nullopt otherwise.
        static std::optional<Json> ParseText(std::string_view input, bool copyData = true, bool checkFinal = true);



        //! @brief Return the element with this index/key if this is an Object or Array. Return std::nullopt otherwise.
        OptRefValWrapper Get(Key key);
        //! @brief Return the element with this index/key if this is an Object or Array. Return std::nullopt otherwise.
        [[nodiscard]] OptCRefValWrapper Get(Key key) const;
        //! @brief Return a copy of the element with this index/key if this is an Object or Array. Return std::nullopt
        //! otherwise.
        [[nodiscard]] OptValWrapper GetCopy(Key key) const;
        //! @brief Move the element with this index/key if this is an Object or Array. Return std::nullopt otherwise.
        OptValWrapper GetAndMove(Key key) &&;


        //! @brief Same as successive calls to Get. Return std::nullopt at the first fail.
        OptRefValWrapper Find(Keys keys);
        //! @brief Same as successive calls to Get. Return std::nullopt at the first fail.
        [[nodiscard]] OptCRefValWrapper Find(Keys keys) const;

        //! @brief Same as successive calls to Get. Return std::nullopt at the first fail.
        RefValWrapperOrNull FindOrNull(Keys keys);
        //! @brief Same as successive calls to Get. Return std::nullopt at the first fail.
        [[nodiscard]] CRefValWrapperOrNull FindOrNull(Keys keys) const;

        //! @brief Same as successive calls to GetCopy. Return std::nullopt at the first fail.
        [[nodiscard]] OptValWrapper FindCopy(Keys keys) const;
        //! @brief Same as successive calls to GetCopy. Return std::nullopt at the first fail.
        [[nodiscard]] ValWrapperOrNull FindOrNullCopy(Keys keys) const;


        //! @brief Same as successive calls to GetCopy. Return std::nullopt at the first fail.
        OptValWrapper FindAndMove(Keys key) &&;
        // //! @brief Same as successive calls to GetCopy. Return std::nullopt at the first fail.
        // ValWrapperOrNull FindOrNullAndMove(Keys key) &&;



        //! @brief Same as successive calls to Get. Return std::nullopt at the first fail.
        template<class Pred = PredicatePointer>
            requires std::predicate<Pred, Json>
        OptRefValWrapper Search(Pred&& pred);
        //! @brief Same as successive calls to Get. Return std::nullopt at the first fail.
        template<class Pred = PredicatePointer>
            requires std::predicate<Pred, Json>
        [[nodiscard]] OptCRefValWrapper Search(Pred&& pred) const;

        //! @brief Same as successive calls to Get. Return std::nullopt at the first fail.
        template<class Pred = PredicatePointer>
            requires std::predicate<Pred, Json>
        RefValWrapperOrNull SearchOrNull(Pred&& pred);
        //! @brief Same as successive calls to Get. Return std::nullopt at the first fail.
        template<class Pred = PredicatePointer>
            requires std::predicate<Pred, Json>
        [[nodiscard]] CRefValWrapperOrNull SearchOrNull(Pred&& pred) const;

        //! @brief Same as successive calls to GetCopy. Return std::nullopt at the first fail.
        template<class Pred = PredicatePointer>
            requires std::predicate<Pred, Json>
        [[nodiscard]] OptValWrapper SearchCopy(Pred&& pred) const;
        //! @brief Same as successive calls to GetCopy. Return std::nullopt at the first fail.
        template<class Pred = PredicatePointer>
            requires std::predicate<Pred, Json>
        [[nodiscard]] ValWrapperOrNull SearchOrNullCopy(Pred&& pred) const;


        //! @brief Same as successive calls to GetCopy. Return std::nullopt at the first fail.
        template<class Pred = PredicatePointer>
            requires std::predicate<Pred, Json>
        OptValWrapper SearchAndMove(Pred&& pred) &&;
        // //! @brief Same as successive calls to GetCopy. Return std::nullopt at the first fail.
        // template<class Pred = PredicatePointer>
        //     requires std::predicate<Pred, Json>
        // ValWrapperOrNull SearchOrNullAndMove(Pred&& pred) &&;


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

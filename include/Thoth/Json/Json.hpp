#pragma once
#include <format>
#include <optional>
#include <expected>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <ranges>

#include <Thoth/Dsa/LinearMap.hpp>
#include <Thoth/Dsa/Cow.hpp>

namespace Thoth::Json {

    struct Json {
        struct JsonVal;

        using Null   = std::monostate;                          // null
        using String = Dsa::Cow<std::string_view, std::string>; // string
        using Number = long double;                             // number
        using Bool   = bool;                                    // bool
        using Object = std::unique_ptr<Json>;                   // {Object}
        using Array  = std::vector<JsonVal>;                    // [Array]


        using JsonKey    = std::string;
        using JsonKeyRef = std::string_view;


        struct JsonVal{
            using Value = std::variant<Null, String, Number, Bool, Object, Array>;

            // NOLINTBEGIN(*)
            JsonVal() = default;
            JsonVal(const Json& child);
            JsonVal(Json&& child);

            JsonVal(Value&& newValue) noexcept;
            JsonVal(const Value& newValue);
            JsonVal(JsonVal&& other) noexcept;
            JsonVal(const JsonVal& other);

            // template<class T>
            //     requires(!std::same_as<std::remove_cvref_t<T>, Json::JsonVal> &&
            //             std::is_constructible_v<Json::JsonVal::Value, T&&>)
            // JsonVal(T&& newValue);

            JsonVal& operator=(const Json& child);
            JsonVal& operator=(Json&& child);

            JsonVal& operator=(Value&& newValue) noexcept;
            JsonVal& operator=(const Value& newValue);
            JsonVal& operator=(JsonVal&& other) noexcept;
            JsonVal& operator=(const JsonVal& other);

            // template<class T>
            //     requires(!std::same_as<std::remove_cvref_t<T>, Json::JsonVal> &&
            //              std::is_constructible_v<Json::JsonVal::Value, T&&>)
            // constexpr JsonVal& operator=(T &&newValue);


            operator Value&();
            operator const Value&() const;
            // NOLINTEND(*)

            template<class T>
            static bool IsOfType(const JsonVal& val);
            template<class T>
            [[nodiscard]] bool IsOf() const;

            template<class T>
            static T& AsType(JsonVal& val);
            template<class T>
            static const T& AsConstType(const JsonVal& val);
            template<class T>
            T& As();
            template<class T>
            const T& As() const;

            bool operator==(const JsonVal& other) const;

            Value value;
        };

        using JsonValRef = JsonVal&;

        using JsonPair = std::pair<JsonKey, JsonVal>;

        using JsonPair    = std::pair<JsonKey, JsonVal>;
        using JsonPairRef = std::pair<JsonKeyRef, JsonValRef>;
        using MapType     = Dsa::LinearMap<JsonKey, JsonVal, std::less<>>;
        // using MapType     = std::map<JsonKey, JsonVal, std::less<>>;

        using IterType   = decltype(MapType{}.begin());
        using CIterType  = decltype(MapType{}.cbegin());


        using RefValWrapper = std::reference_wrapper<JsonVal>;
        using CRefValWrapper = std::reference_wrapper<const JsonVal>;

        using OptRefValWrapper = std::optional<RefValWrapper>;
        using OptCRefValWrapper = std::optional<CRefValWrapper>;

        using RefValWrapperOrNull = std::expected<RefValWrapper, Null>;
        using CRefValWrapperOrNull = std::expected<CRefValWrapper, Null>;


        Json() = default;
        Json(const Json& other);
        Json(Json&& other) noexcept = default;

        Json& operator=(const Json& other);
        Json& operator=(Json&& other) noexcept = default;


        //! @brief Create with an existing map.
        explicit Json(MapType&& initAs);

        Json(const std::initializer_list<JsonPair>& init);


        //! @brief Tries to parse the Json from a string.
        //! @param text the text to parse.
        //! @param copyData if this is false then the json will use references to text.
        //! @return A Json if the parse success, std::nullopt if it fails.
        static std::optional<Json> Parse(std::string_view text, bool copyData = true);


        //! @brief check if a key exists.
        //! @param key The key to be checked.
        //! @return True if the key exists, false otherwise.
        [[nodiscard]] bool Exists(JsonKeyRef key) const;

        //! @brief check if a key exists.
        //! @param p A pair with the key and value to be checked.
        //! @return True if the key exists, false otherwise.
        [[nodiscard]] bool Exists(JsonPairRef p) const;

        //! @brief check if a key=val exists.
        //! @param key The key to be checked.
        //! @param val The value to be checked.
        //! @return True if the key-value pair exists, false otherwise.
        [[nodiscard]] bool Exists(JsonKeyRef key, JsonValRef val) const;
        //! @brief Add a value with the specified key. Replace if already exists.
        //! @param p A pair with the key and the value to be added.
        void Set(JsonPairRef p);

        //! @brief same as @ref Add(JsonPairRef) "Add(JsonPairRef p)".
        void Set(JsonKeyRef key, JsonValRef val);

        //! @brief Removes a key.
        bool Remove(JsonKeyRef key);

        //! @brief If key not exists, set it to value.
        //! @param p A pair with the key and the value to be added.
        //! @return True if the key not exists, false otherwise.
        bool SetIfNull(JsonPairRef p);


        //! @brief same as @ref SetIfNull(JsonPairRef) "SetIfNull(JsonPairRef p)".
        bool SetIfNull(JsonKeyRef key, JsonValRef val);

        //! @brief Get the reference of a key but don't create if it not exists.
        //! @param key The key.
        //! @return std::reference_wrapper<JsonVal> if the key exists, std::nullopt otherwise.
        OptRefValWrapper Get(JsonKeyRef key);

        //! @brief Get the reference of a key but don't create if it not exists.
        //! @param key The key.
        //! @return std::reference_wrapper<const JsonVal> if the key exists, std::nullopt otherwise.
        [[nodiscard]] OptCRefValWrapper Get(JsonKeyRef key) const;


        //! @brief Get the reference of a key or return null if it not exists.
        //! @param key The key.
        //! @return std::reference_wrapper<const JsonVal> if the key exists, NullV otherwise.
        [[nodiscard]] CRefValWrapperOrNull GetOrNull(JsonKeyRef key) const;

        //! @brief Same as successive calls to Get. Return std::nullopt at the first fail.
        template<std::ranges::range R>
        OptRefValWrapper NestedFind(R&& keys);

        //! @brief Same as successive calls to Get. Return std::nullopt at the first fail.
        template<std::ranges::range R>
        OptCRefValWrapper NestedFind(R&& keys) const;

        //! @brief Same as successive calls to Get. Return std::nullopt at the first fail.
        template<std::ranges::range R>
        RefValWrapperOrNull NestedFindOrNull(R&& keys);

        //! @brief Same as successive calls to Get. Return std::nullopt at the first fail.
        template<std::ranges::range R>
        CRefValWrapperOrNull NestedFindOrNull(R&& keys) const;


        IterType begin()                      { return pairs.begin(); }
        IterType end()                        { return pairs.end(); }
        [[nodiscard]] CIterType begin() const { return pairs.cbegin(); }
        [[nodiscard]] CIterType end() const   { return pairs.cend(); }



        //! @brief Clear all keys.
        void Clear();

        //! @return The count of keys.
        [[nodiscard]] size_t Size() const;

        //! @return True if Size() is 0.
        [[nodiscard]] bool Empty() const;



            //! @return The JsonVal& associated with a key. Create if it not exists.
            //! STL containers has many problems so it must be JsonKey.
        JsonVal& operator[](JsonKeyRef key);
            //! @return The const JsonVal& associated with a key or returns a null reference if it not exists.
            //! STL containers has many problems so it must be JsonKey.
        const JsonVal& operator[](JsonKeyRef key) const;

        //! @return True if both jsons match.
        bool operator==(const Json& other) const;

        static bool ParseUnchecked(std::string_view& input, Json& json);
    private:
        std::string buffer;
        std::string_view bufferView;

        MapType pairs;

        friend struct std::formatter<Json>;
    };

    using String  = Json::String;
    using Number  = Json::Number;
    using Bool    = Json::Bool;
    using Object  = Json::Object;
    using Array   = Json::Array;
    using Null    = Json::Null;

    inline static constexpr Null NullV{};
    inline static Json::JsonVal NullJ{}; // erro!
}


#include <Thoth/Json/Json.tpp>
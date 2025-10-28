#pragma once
#include <format>
#include <optional>
#include <expected>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <map>

#ifdef __cpp_lib_constexpr_map
// std::map isn't constexpr in MSVC C++ yet
#define CONSTEXPR_WHEN_MSVC_STARTS_WORKING constexpr
#else
// std::map isn't constexpr in MSVC C++ yet
#define CONSTEXPR_WHEN_MSVC_STARTS_WORKING
#endif


namespace Thoth::Json {

    struct Json {
        struct JsonVal;

        using String  = std::string;           // string
        using Number  = long double;           // number
        using Bool    = bool;                  // bool
        using Object  = std::unique_ptr<Json>; // {Object}
        using Array   = std::vector<JsonVal>;  // [Array]
        using Null    = std::monostate;        // null


        using JsonKey  = const std::string;
        using JsonKeyRef  = const std::string_view;


        struct JsonVal{
            using Value = std::variant<String, Number, Bool, Object, Array, Null>;

            // NOLINTBEGIN(*)
            CONSTEXPR_WHEN_MSVC_STARTS_WORKING JsonVal(const Json& child);
            CONSTEXPR_WHEN_MSVC_STARTS_WORKING JsonVal(Json&& child);

            CONSTEXPR_WHEN_MSVC_STARTS_WORKING JsonVal(Value&& newValue);
            CONSTEXPR_WHEN_MSVC_STARTS_WORKING JsonVal(const Value& newValue);
            CONSTEXPR_WHEN_MSVC_STARTS_WORKING JsonVal(JsonVal&& other) noexcept;
            CONSTEXPR_WHEN_MSVC_STARTS_WORKING JsonVal(const JsonVal& other);

            template<class T>
            CONSTEXPR_WHEN_MSVC_STARTS_WORKING JsonVal(T newValue);
            // NOLINTEND(*)


            CONSTEXPR_WHEN_MSVC_STARTS_WORKING JsonVal& operator=(JsonVal&& other) noexcept;
            CONSTEXPR_WHEN_MSVC_STARTS_WORKING JsonVal& operator=(const JsonVal& other);

            CONSTEXPR_WHEN_MSVC_STARTS_WORKING operator Value&();
            CONSTEXPR_WHEN_MSVC_STARTS_WORKING operator const Value&() const;

            template<class T>
            CONSTEXPR_WHEN_MSVC_STARTS_WORKING bool IsOf() const;
            template<class T>
            CONSTEXPR_WHEN_MSVC_STARTS_WORKING T& As();
            template<class T>
            CONSTEXPR_WHEN_MSVC_STARTS_WORKING const T& As() const;

            CONSTEXPR_WHEN_MSVC_STARTS_WORKING bool operator==(const JsonVal &) const = default;

            Value value;
        };

        using JsonValRef = JsonVal&;




        using JsonPair = std::pair<JsonKey, JsonVal>;

        using JsonPair     = std::pair<JsonKey, JsonVal>;
        using JsonPairRef  = std::pair<JsonKeyRef, JsonValRef>;
        using MapType        = std::map<JsonKey, JsonVal, std::less<>>;

        using IterType       = decltype(MapType{}.begin());
        using CIterType      = decltype(MapType{}.cbegin());
        using RIterType      = decltype(MapType{}.rbegin());
        using CRIterType     = decltype(MapType{}.crbegin());

        CONSTEXPR_WHEN_MSVC_STARTS_WORKING Json() = default;
        CONSTEXPR_WHEN_MSVC_STARTS_WORKING Json(const Json& other) = default;
        CONSTEXPR_WHEN_MSVC_STARTS_WORKING Json(Json&& other) noexcept = default;


        //! @brief Create with an existing map.
        CONSTEXPR_WHEN_MSVC_STARTS_WORKING explicit Json(MapType&& initAs);

        CONSTEXPR_WHEN_MSVC_STARTS_WORKING Json(const std::initializer_list<JsonPair>& init);


    //! @brief Tries to parse the Json from a string.
    //! @param text the text to parse.
    //! @return A Json if the parse success, std::nullopt if it fails.
    static std::optional<Json> Parse(std::string_view text);


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
        std::optional<std::reference_wrapper<JsonVal>> Get(JsonKeyRef key);

        //! @brief Get the reference of a key but don't create if it not exists.
        //! @param key The key.
        //! @return std::reference_wrapper<const JsonVal> if the key exists, std::nullopt otherwise.
        [[nodiscard]] std::optional<std::reference_wrapper<const JsonVal>> Get(JsonKeyRef key) const;


        //! @brief Get the reference of a key or return null if it not exists.
        //! @param key The key.
        //! @return std::reference_wrapper<JsonVal> if the key exists, NullV otherwise.
        std::expected<std::reference_wrapper<JsonVal>, Null> GetOrNull(JsonKeyRef key);

        //! @brief Get the reference of a key or return null if it not exists.
        //! @param key The key.
        //! @return std::reference_wrapper<const JsonVal> if the key exists, NullV otherwise.
        std::expected<std::reference_wrapper<const JsonVal>, Null> GetOrNull(JsonKeyRef key) const;


    IterType begin()                      { return pairs.begin(); }
    IterType end()                        { return pairs.end(); }
    [[nodiscard]] CIterType begin() const { return pairs.cbegin(); }
    [[nodiscard]] CIterType end() const   { return pairs.cend(); }

    RIterType rbegin()                      { return pairs.rbegin(); }
    RIterType rend()                        { return pairs.rend(); }
    [[nodiscard]] CRIterType rbegin() const { return pairs.rbegin(); }
    [[nodiscard]] CRIterType rend() const   { return pairs.rend(); }



    //! @brief Clear all keys.
    void Clear();

    //! @return The count of keys.
    [[nodiscard]] size_t Size() const;

    //! @return True if Size() is 0.
    [[nodiscard]] bool Empty() const;



        //! @return The JsonVal& associated with a key. Create if it not exists.
        //! STL containers has many problems so it must be JsonKey.
    JsonVal& operator[](JsonKeyRef key);
        //! @return The const JsonVal& associated with a key or returns an null reference if it not exists.
        //! STL containers has many problems so it must be JsonKey.
    const JsonVal& operator[](JsonKeyRef key) const;

    //! @return True if both jsons match.
    bool operator==(const Json& other) const = default;
    private:
        MapType pairs;

        static std::optional<Json>
            ParseUnchecked(std::string_view::const_iterator &itBegin,  const std::string_view::const_iterator& itEnd);
        friend struct std::formatter<Json>;
    };

    using String  = Json::String;
    using Number  = Json::Number;
    using Bool    = Json::Bool;
    using Object  = Json::Object;
    using Array   = Json::Array;
    using Null    = Json::Null;

    static constexpr Null NullV{};
    static CONSTEXPR_WHEN_MSVC_STARTS_WORKING Json::JsonVal NullJ{ NullV };
}


#include <Thoth/Json/Json.tpp>
// #undef CONSTEXPR_WHEN_MSVC_STARTS_WORKING
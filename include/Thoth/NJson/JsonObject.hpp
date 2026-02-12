#pragma once
#include <optional>
#include <format>
#include <memory>
#include <string>
#include <vector>

#include <Thoth/Dsa/LinearMap.hpp>


namespace Thoth::NJson {
    using JsonObjKey    = std::string;
    using JsonObjKeyRef = std::string_view;

    struct JsonObject {
        using JsonValRef = Json&;

        using JsonPair    = std::pair<JsonObjKey, Json>;
        using JsonPairRef = std::pair<JsonObjKeyRef, JsonValRef>;
        using MapType     = Dsa::LinearMap<JsonObjKey, Json>;

        using IterType   = decltype(MapType{}.begin());
        using CIterType  = decltype(MapType{}.cbegin());

        ~JsonObject();

        JsonObject() = default;
        JsonObject(JsonObject&& other) noexcept;
        JsonObject(const JsonObject& other);

        //! @brief Create with an existing map.
        explicit JsonObject(MapType&& initAs);

        // NOLINTNEXTLINE(*)
        JsonObject(std::initializer_list<JsonPair> init);


        JsonObject& operator=(JsonObject&& other) noexcept;
        JsonObject& operator=(const JsonObject& other);

        JsonObject& operator=(std::initializer_list<JsonPair> list);

        //! @brief check if a key exists.
        //! @param key The key to be checked.
        //! @return True if the key exists, false otherwise.
        [[nodiscard]] bool Exists(JsonObjKeyRef key) const;

        //! @brief check if a key exists.
        //! @param p A pair with the key and value to be checked.
        //! @return True if the key exists, false otherwise.
        [[nodiscard]] bool Exists(JsonPairRef p) const;

        //! @brief check if a key=val exists.
        //! @param key The key to be checked.
        //! @param val The value to be checked.
        //! @return True if the key-value pair exists, false otherwise.
        [[nodiscard]] bool Exists(JsonObjKeyRef key, JsonValRef val) const;
        //! @brief Add a value with the specified key. Replace if already exists.
        //! @param p A pair with the key and the value to be added.
        void Set(JsonPairRef p);

        //! @brief same as @ref Add(JsonPairRef) "Add(JsonPairRef p)".
        void Set(JsonObjKeyRef key, JsonValRef val);

        //! @brief Removes a key.
        bool Remove(JsonObjKeyRef key);

        //! @brief If key not exists, set it to value.
        //! @param p A pair with the key and the value to be added.
        //! @return True if the key not exists, false otherwise.
        bool SetIfNull(JsonPairRef p);


        //! @brief same as @ref SetIfNull(JsonPairRef) "SetIfNull(JsonPairRef p)".
        bool SetIfNull(JsonObjKeyRef key, JsonValRef val);

        //! @brief Get the reference of a key but don't create if it not exists.
        //! @param key The key.
        //! @return JsonVal* if the key exists, std::nullopt otherwise.
        OptRefValWrapper Get(JsonObjKeyRef key);

        //! @brief Get the reference of a key but don't create if it not exists.
        //! @param key The key.
        //! @return const JsonVal* if the key exists, std::nullopt otherwise.
        [[nodiscard]] OptCRefValWrapper Get(JsonObjKeyRef key) const;


        //! @brief Get the reference of a key or return null if it not exists.
        //! @param key The key.
        //! @return const JsonVal* if the key exists, NullV otherwise.
        [[nodiscard]] CRefValWrapperOrNull GetOrNull(JsonObjKeyRef key) const;

        //! @brief Copy of a value if it exists.
        //! @param key The key.
        //! @return const JsonVal* if the key exists, std::nullopt otherwise.
        [[nodiscard]] OptValWrapper GetCopy(JsonObjKeyRef key) const;

        //! @brief Copy of a value or return null if it not exists.
        //! @param key The key.
        //! @return const JsonVal* if the key exists, NullV otherwise.
        [[nodiscard]] ValWrapperOrNull GetOrNullCopy(JsonObjKeyRef key) const;

        //! @brief Copy of a value if it exists.
        //! @param key The key.
        //! @return const JsonVal* if the key exists, std::nullopt otherwise.
        OptValWrapper GetAndMove(JsonObjKeyRef key) &&;

        //! @brief Copy of a value or return null if it not exists.
        //! @param key The key.
        //! @return const JsonVal* if the key exists, NullV otherwise.
        ValWrapperOrNull GetOrNullAndMove(JsonObjKeyRef key) &&;



        IterType begin()                      { return _pairs.begin(); }
        IterType end()                        { return _pairs.end(); }
        [[nodiscard]] CIterType begin() const { return _pairs.cbegin(); }
        [[nodiscard]] CIterType end() const   { return _pairs.cend(); }



        //! @brief Clear all keys.
        void Clear();

        //! @return The count of keys.
        [[nodiscard]] size_t Size() const;

        //! @return True if Size() is 0.
        [[nodiscard]] bool Empty() const;



        //! @return The JsonVal& associated with a key. Create if it not exists.
        //! STL containers has many problems so it must be JsonKey.
        Json& operator[](JsonObjKeyRef key);
        //! @return The const JsonVal& associated with a key or returns a null reference if it not exists.
        //! STL containers has many problems so it must be JsonKey.
        const Json& operator[](JsonObjKeyRef key) const;

        //! @return True if both jsons match.
        bool operator==(const JsonObject& other) const;

        friend Json;
        friend bool Details::ReadObject(std::string_view& input, auto& val, const Details::BufferInfo& info);
    private:
        MapType _pairs{};

        friend struct std::formatter<JsonObject>;
    };
}

#include <Thoth/NJson/JsonObject.tpp>
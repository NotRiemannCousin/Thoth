#pragma once
#include <optional>
#include <format>
#include <vector>
#include <string>

namespace Thoth::Http {
    struct HttpHeaders {
        using HeaderKey      = std::string;
        using HeaderKeyRef   = const std::string&;

        using HeaderValue    = std::string;
        using HeaderValueRef = const std::string&;

        // Well, it really maps to something, but isn't a map. The name will be maintained
        // to not break the naming convention of this lib.
        using HeaderPair     = std::pair<HeaderKey, HeaderValue>;
        using HeaderPairRef  = std::pair<HeaderKeyRef, HeaderValueRef>;
        using MapType        = std::vector<HeaderPair>;

        using IterType       = decltype(MapType{}.begin());
        using CIterType      = decltype(MapType{}.cbegin());
        using RIterType      = decltype(MapType{}.rbegin());
        using CRIterType     = decltype(MapType{}.crbegin());


        //! @brief Create with an existing vector.
        explicit HttpHeaders(const MapType& initAs);

        HttpHeaders(const std::initializer_list<HeaderPair>& init);


        static std::optional<HttpHeaders> Parse(std::string_view headers);


        //! @brief check if a key exists.
        //! @param key The key to be checked.
        //! @return True if the key exists, false otherwise.
        [[nodiscard]] bool Exists(HeaderKeyRef key) const;

        //! @brief check if a key exists.
        //! @param p A pair with the key and value to be checked.
        //! @return True if the key exists, false otherwise.
        [[nodiscard]] bool Exists(HeaderPairRef p) const;

        //! @brief check if a key=val exists.
        //! @param key The key to be checked.
        //! @param val The value to be checked.
        //! @return True if the key-value pair exists, false otherwise.
        [[nodiscard]] bool Exists(HeaderKeyRef key, HeaderValueRef val) const;

        //! @brief Add a value with the specified key. Append if already exists.
        //! @param p A pair with the key and the value to be added.
        void Add(HeaderPairRef p);

        //! @brief same as @ref Add(HeaderPairRef) "Add(HeaderPairRef p)".
        void Add(HeaderKeyRef key, HeaderValueRef val);


        //! @brief Add a value with the specified key. Replace if already exists.
        //! @param p A pair with the key and the value to be added.
        void Set(HeaderPairRef p);

        //! @brief same as @ref Add(HeaderPairRef) "Add(HeaderPairRef p)".
        void Set(HeaderKeyRef key, HeaderValueRef val);

        //! @brief Remove a value with the specified key.
        //! @param p A pair with the key and the value to be removed.
        //! @return True if the key exists, false otherwise.
        bool Remove(HeaderPairRef p);

        //! @brief same as @ref Remove(HeaderPairRef) "Remove(HeaderPairRef p)".
        bool Remove(HeaderKeyRef key, HeaderValueRef val);

        //! @brief If key not exists, set it to value.
        //! @param p A pair with the key and the value to be added.
        //! @return True if the key not exists, false otherwise.
        bool SetIfNull(HeaderPairRef p);


        //! @brief same as @ref SetIfNull(HeaderPairRef) "SetIfNull(HeaderPairRef p)".
        bool SetIfNull(HeaderKeyRef key, HeaderValueRef val);

        //! @brief Get the reference of a key but don't create if it not exists.
        //! @param key The key.
        //! @return std::reference_wrapper<HeaderValue> if the key exists, std::nullopt otherwise.
        std::optional<std::reference_wrapper<HeaderValue>> Get(HeaderKeyRef key);

        //! @brief Get the reference of a key but don't create if it not exists.
        //! @param key The key.
        //! @return std::reference_wrapper<const HeaderValue> if the key exists, std::nullopt otherwise.
        std::optional<std::reference_wrapper<const HeaderValue>> Get(HeaderKeyRef key) const;


        //! @brief Get all Set-Cookie header values as they cannot be comma-separated.
        //! @return A readonly view of all Set-Cookie values. No Copies => risk of dangling reference, keep it in mind.
        auto GetSetCookieView() const;

        //! @brief Get all Set-Cookie header values as they cannot be comma-separated.
        //! @return Vector of all Set-Cookie values.
        std::vector<HeaderValue> GetSetCookie() const;



        IterType begin();
        IterType end();
        [[nodiscard]] CIterType begin() const;
        [[nodiscard]] CIterType end() const;

        RIterType rbegin();
        RIterType rend();
        [[nodiscard]] CRIterType rbegin() const;
        [[nodiscard]] CRIterType rend() const;



        //! @brief Clear all keys.
        void Clear();

        //! @return The count of keys.
        [[nodiscard]] size_t Size() const;

        //! @return True if Size() is 0.
        [[nodiscard]] bool Empty() const;



        //! @return The HeaderValue& associated with a key. Create if it not exists.
        //! STL containers has many problems so it must be HeaderKey.
        HeaderValue& operator[](HeaderKeyRef key);

        //! @return True if both headers match.
        bool operator==(const HttpHeaders& other) const;
    private:
        MapType _headers;

        friend struct std::formatter<HttpHeaders>;
    };
}

#include <Thoth/Http/HttpHeaders.tpp>
#pragma once
#include <Thoth/Http/Response/HttpStatusCodeEnum.hpp>
#include <optional>
#include <format>
#include <vector>
#include <string>
#include <ranges>

namespace Thoth::Http {
    //! @brief HttpHeaders is a dumb class that stores the headers. Although it can parse the raw headers from TCP,
    //! the role of this class is just store it, so it can't check if the headers individually are well-formed.
    struct HttpHeaders {
        using HeaderKey      = std::string;
        using HeaderKeyRef   = std::string_view;

        using HeaderValue    = std::string;
        using HeaderValueRef = std::string_view;

        // Well, it really maps to something, but isn't a map. The name will be maintained
        // to not break the naming convention of this lib.
        using HeaderPair     = std::pair<HeaderKey, HeaderValue>;
        using HeaderPairRef  = std::pair<HeaderKeyRef, HeaderValueRef>;
        using MapType        = std::vector<HeaderPair>;

        using IterType       = decltype(MapType{}.begin());
        using CIterType      = decltype(MapType{}.cbegin());
        using RIterType      = decltype(MapType{}.rbegin());
        using CRIterType     = decltype(MapType{}.crbegin());


        HttpHeaders();

        //! @brief Create with an existing vector.
        explicit HttpHeaders(const MapType& initAs);

        HttpHeaders(const std::initializer_list<HeaderPair>& init);


        //! @brief Tries to parse the headers from the raw TCP string.
        //! @param headers the headers separated by  "\r\n".
        //! @param maxHeadersLength the max length that the headers can achieve.
        //! @return A HttpHeaders if the parse success, @ref "bad request" HttpStatusCodeEnum::BAD_REQUEST if the parse
        //! fails and @ref "content too large" HttpStatusCodeEnum::CONTENT_TOO_LARGE if the header is too long.
        template<std::ranges::input_range R>
        static WebResult<HttpHeaders> Parse(R& headers, size_t maxHeadersLength = 1<<16);


        static HttpHeaders DefaultHeaders();


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
        [[nodiscard]] std::optional<std::reference_wrapper<const HeaderValue>> Get(HeaderKeyRef key) const;


        //! @brief Get all Set-Cookie header values as they cannot be comma-separated.
        //! @return A readonly view of all Set-Cookie values. No Copies => risk of dangling reference, keep it in mind.
        [[nodiscard]] auto GetSetCookieView() const;

        //! @brief Get all Set-Cookie header values as they cannot be comma-separated.
        //! @return Vector of all Set-Cookie values.
        [[nodiscard]] std::vector<HeaderValue> GetSetCookie() const;



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
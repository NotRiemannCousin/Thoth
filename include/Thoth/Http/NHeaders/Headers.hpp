#pragma once
#include <chrono>
#include <Thoth/Http/Response/StatusCodeEnum.hpp>
#include <optional>
#include <format>
#include <vector>
#include <ranges>

#include <Thoth/Http/Request/Url.hpp>
#include <Thoth/Http/NHeaders/_base.hpp>

#include <Thoth/Http/NHeaders/Headers/_pch.hpp>
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>

namespace Thoth::Http::NHeaders {
    template<bool IsConst, Serializable ...T>
    struct ListProxy;
    template<bool IsConst, Serializable ...T>
    struct ValueProxy;
}


namespace Thoth::Http {

    enum class VersionEnum : uint8_t;


    //! @brief This class stores the headers from HTTP.
    struct Headers {
        using HeaderKey      = NHeaders::HeaderKey;
        using HeaderKeyRef   = NHeaders::HeaderKeyRef;

        using HeaderValue    = NHeaders::HeaderValue;
        using HeaderValueRef = NHeaders::HeaderValueRef;

        using HeaderPair     = NHeaders::HeaderPair;
        using HeaderPairRef  = NHeaders::HeaderPairRef;
        using MapType        = NHeaders::MapType;

        using MapType        = std::vector<HeaderPair>;

        using IterType       = decltype(MapType{}.begin());
        using CIterType      = decltype(MapType{}.cbegin());
        using RIterType      = decltype(MapType{}.rbegin());
        using CRIterType     = decltype(MapType{}.crbegin());


        Headers();

        //! @brief Create with an existing vector.
        explicit Headers(const MapType& initAs);

        Headers(std::initializer_list<HeaderPair> init);


        //! @brief Tries to parse the headers from the raw TCP std::string.
        //! @param headers the headers separated by  "\r\n".
        //! @param maxHeadersLength the max length that the headers can achieve.
        //! @return A Headers if the parse success, @ref "bad request" StatusCodeEnum::BAD_REQUEST if the parse
        //! fails and @ref "content too large" StatusCodeEnum::CONTENT_TOO_LARGE if the header is too long.
        template<std::ranges::input_range R>
        static WebResult<Headers> Parse(R& headers, size_t maxHeadersLength = 1<<16);


        static Headers DefaultHeaders();


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
        //! @return HeaderValue* if the key exists, std::nullopt otherwise.
        std::optional<HeaderValue*> Get(HeaderKeyRef key);

        //! @brief Get the reference of a key but don't create if it not exists.
        //! @param key The key.
        //! @return const HeaderValue* if the key exists, std::nullopt otherwise.
        [[nodiscard]] std::optional<const HeaderValue*> Get(HeaderKeyRef key) const;


        //! @{
        //! @name Proxies
        //! Convenient calls to some headers.

        //! @brief Accept-Encoding header (gzip, br, etc).
        NHeaders::ListProxy<false, NHeaders::MimeType> Accept();
        NHeaders::ListProxy<true, NHeaders::MimeType> Accept() const;

        //! @brief Accept-Encoding header (gzip, br, etc).
        NHeaders::ListProxy<false, NHeaders::AcceptEncodingEnum> AcceptEncoding();
        NHeaders::ListProxy<true, NHeaders::AcceptEncodingEnum> AcceptEncoding() const;


        //! @brief Defines the media type of the resource (MIME).
        NHeaders::ValueProxy<false, NHeaders::MimeType> ContentType();
        //! @copybrief ContentType
        [[nodiscard]] NHeaders::ValueProxy<true, NHeaders::MimeType> ContentType() const;

        //! @brief The size of the entity-body in bytes.
        NHeaders::ValueProxy<false, uint64_t> ContentLength();
        //! @copybrief ContentLength
        [[nodiscard]] NHeaders::ValueProxy<true, uint64_t> ContentLength() const;

        //! @brief List of encodings (compression) applied to the entity.
        NHeaders::ListProxy<false, NHeaders::ContentEncodingEnum> ContentEncoding();
        //! @copybrief ContentEncoding
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::ContentEncodingEnum> ContentEncoding() const;

        //! @brief List of compression applied to the entity.
        NHeaders::ListProxy<false, NHeaders::TransferEncodingEnum> TransferEncoding();
        //! @copybrief TransferEncoding
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::TransferEncodingEnum> TransferEncoding() const;

        //! @brief Natural languages for the intended audience (e.g., "en-US").
        NHeaders::ListProxy<false, std::string> ContentLanguage();
        //! @copybrief ContentLanguage
        [[nodiscard]] NHeaders::ListProxy<true, std::string> ContentLanguage() const;

        //! @brief The specific location for the entity-body.
        NHeaders::ValueProxy<false, std::string> ContentLocation();
        //! @copybrief ContentLocation
        [[nodiscard]] NHeaders::ValueProxy<true, std::string> ContentLocation() const;

        //! @brief Date and time at which the message was originated.
        NHeaders::ValueProxy<false, std::chrono::utc_clock::time_point> Date();
        //! @copybrief Date
        [[nodiscard]] NHeaders::ValueProxy<true, std::chrono::utc_clock::time_point> Date() const;

        //! @brief Options for the current connection.
        NHeaders::ListProxy<false, std::string> Connection();
        //! @copybrief Connection
        [[nodiscard]] NHeaders::ListProxy<true, std::string> Connection() const;

        //! @brief Used to signal a protocol change (e.g., "websocket").
        NHeaders::ListProxy<false, NHeaders::Upgrade> Upgrade();
        //! @copybrief Upgrade
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::Upgrade> Upgrade() const;

        //! @brief Indicates header fields present in the trailer of a chunked message.
        NHeaders::ListProxy<false, std::string> Trailer();
        //! @copybrief Trailer
        [[nodiscard]] NHeaders::ListProxy<true, std::string> Trailer() const;

        //! @brief Path taken by the request/response through proxies (free std::string).
        NHeaders::ListProxy<false, std::string> Via();
        //! @copybrief Via
        [[nodiscard]] NHeaders::ListProxy<true, std::string> Via() const;

        //! @}



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
        bool operator==(const Headers& other) const;
    private:
        MapType _headers;

        friend struct std::formatter<Headers>;
    };
}

#include <Thoth/Http/NHeaders/Headers.tpp>
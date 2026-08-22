#pragma once
#include <chrono>
#include <Thoth/Http/Response/StatusCodeEnum.hpp>
#include <optional>
#include <format>
#include <vector>
#include <ranges>
#include <string_view>
#include <utility>

#include <Thoth/Http/Url/Url.hpp>
#include <Thoth/Http/NHeaders/_base.hpp>

#include <Thoth/Http/NHeaders/Headers/_pch.hpp>
#include <Thoth/Http/NHeaders/Proxy/_base.hpp>
#include <Thoth/Http/NHeaders/_base/Weighted.hpp>
#include <Thoth/Http/NHeaders/_base/Parameterized.hpp>

namespace Thoth::Http::NHeaders {
    template<bool IsConst, Utils::Serializable ...T>
    struct ListProxy;
    template<bool IsConst, Utils::Serializable ...T>
    struct ValueProxy;
    template<bool IsConst, Utils::Serializable ...T>
    struct MultiValueProxy;
}


namespace Thoth::Http {

    enum class VersionEnum : uint8_t;

    struct RequestHeaders;
    struct ResponseHeaders;

    bool InsensitiveCmp(std::string_view elem1, std::string_view elem2);
    bool IsSingleValue(std::string_view key);

    //! @brief Stores the common header fields of an HTTP message.
    //!
    //! Its typed accessors expose concrete models from `NHeaders`, including `MimeType`, `AcceptEncoding`,
    //! `ContentEncodingEnum`, `TransferEncodingEnum`, `Upgrade` and `Link`.
    //!
    //! @par Example
    //! @code{.cpp}
    //! Headers headers;
    //! auto json{ NHeaders::MimeTypes::appJson.WithParam("charset", "utf-8") };
    //! headers.ContentType().Set(json);
    //! headers.ContentEncoding().Add(NHeaders::ContentEncodingEnum::Gzip);
    //! auto length{ headers.ContentLength().GetWithDefault(std::uint64_t{ 0 }) };
    //! @endcode
    struct Headers {
        using HeaderKey      = NHeaders::HeaderKey;
        using HeaderKeyRef   = NHeaders::HeaderKeyRef;

        using HeaderValue    = NHeaders::HeaderValue;
        using HeaderValueRef = NHeaders::HeaderValueRef;

        using HeaderPair     = NHeaders::HeaderPair;
        using HeaderPairRef  = NHeaders::HeaderPairRef;
        using HeaderRef      = HeaderPair*;
        using ConstHeaderRef = const HeaderPair*;
        using MapType        = NHeaders::MapType;

        using IterType       = decltype(MapType{}.begin());
        using CIterType      = decltype(MapType{}.cbegin());
        using RIterType      = decltype(MapType{}.rbegin());
        using CRIterType     = decltype(MapType{}.crbegin());


        Headers();


        //! @brief Create with an existing vector.
        explicit Headers(const MapType& initAs);

        Headers(std::initializer_list<HeaderPair> init);
        Headers(std::initializer_list<std::initializer_list<HeaderPair>>) = delete;


        //! @brief Tries to parse the headers from the raw TCP std::string.
        //! @param headers the headers separated by  "\r\n".
        //! @param maxHeadersLength the max length that the headers can achieve.
        //! @return A Headers if the parse success, @ref "bad request" StatusCodeEnum::BAD_REQUEST if the parse
        //! fails and @ref "content too large" StatusCodeEnum::CONTENT_TOO_LARGE if the header is too long.
        template<std::ranges::input_range R>
        static std::expected<Headers, MessageParseErrorEnum> Parse(R&& headers, size_t maxHeadersLength = 1<<16);


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
        //! @param k A The key to be removed.
        bool Remove(HeaderKeyRef k);

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

        //! @brief Get all the references associated with a key.
        //! @param key The key.
        //! @return HeaderRef for every matching header.
        std::vector<HeaderRef> GetAll(HeaderKeyRef key);

        //! @copydoc GetAll
        [[nodiscard]] std::vector<ConstHeaderRef> GetAll(HeaderKeyRef key) const;


        //! @{
        //! @name Proxies
        //! Convenient calls to some headers.

        //! @brief Accept header, media types accepted with an optional preference weight.
        //! @par Example
        //! @code{.cpp}
        //! auto html{ NHeaders::MimeTypes::textHtml.WithParam("charset", "utf-8") };
        //! headers.Accept().Add(html);
        //! @endcode
        NHeaders::ListProxy<false, NHeaders::MimeType> Accept();
        //! @copydoc Accept
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::MimeType> Accept() const;

        //! @brief Accept-Encoding header (gzip, br, etc), with an optional preference weight.
        //! @par Example
        //! @code{.cpp}
        //! auto br{ NHeaders::AcceptEncoding{ NHeaders::AcceptEncodingEnum::Br } };
        //! headers.AcceptEncoding().Add(br.WithWeight(0.8));
        //! @endcode
        NHeaders::ListProxy<false, NHeaders::AcceptEncoding> AcceptEncoding();
        //! @copydoc AcceptEncoding
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::AcceptEncoding> AcceptEncoding() const;


        //! @brief Defines the media type of the resource (MIME).
        //! @par Example
        //! @code{.cpp}
        //! headers.ContentType().Set(
        //!     NHeaders::MimeTypes::appJson.WithParam("charset", "utf-8"));
        //! @endcode
        NHeaders::ValueProxy<false, NHeaders::MimeType> ContentType();
        //! @copydoc ContentType
        [[nodiscard]] NHeaders::ValueProxy<true, NHeaders::MimeType> ContentType() const;

        //! @brief The size of the entity-body in bytes.
        //! @par Example
        //! @code{.cpp}
        //! auto length{ headers.ContentLength().GetWithDefault(std::uint64_t{ 0 }) };
        //! @endcode
        NHeaders::ValueProxy<false, uint64_t> ContentLength();
        //! @copydoc ContentLength
        [[nodiscard]] NHeaders::ValueProxy<true, uint64_t> ContentLength() const;

        //! @brief List of encodings (compression) applied to the entity.
        //! @par Example
        //! @code{.cpp}
        //! headers.ContentEncoding().Add(NHeaders::ContentEncodingEnum::Gzip);
        //! @endcode
        NHeaders::ListProxy<false, NHeaders::ContentEncodingEnum> ContentEncoding();
        //! @copydoc ContentEncoding
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::ContentEncodingEnum> ContentEncoding() const;

        //! @brief List of compression applied to the entity.
        //! @par Example
        //! @code{.cpp}
        //! headers.TransferEncoding().Set(
        //!     std::vector{ NHeaders::TransferEncodingEnum::Chunked });
        //! @endcode
        NHeaders::ListProxy<false, NHeaders::TransferEncodingEnum> TransferEncoding();
        //! @copydoc TransferEncoding
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::TransferEncodingEnum> TransferEncoding() const;

        //! @brief Natural languages for the intended audience (e.g., "en-US").
        //! @par Example
        //! @code{.cpp}
        //! headers.ContentLanguage().Set(std::vector<std::string>{ "pt-BR", "en-US" });
        //! auto languages{ headers.ContentLanguage().GetAsOpt() };
        //! @endcode
        NHeaders::ListProxy<false, std::string> ContentLanguage();
        //! @copydoc ContentLanguage
        [[nodiscard]] NHeaders::ListProxy<true, std::string> ContentLanguage() const;

        //! @brief The specific location for the entity-body.
        //! @par Example
        //! @code{.cpp}
        //! headers.ContentLocation().TrySet("/documents/42");
        //! @endcode
        NHeaders::ValueProxy<false, std::string> ContentLocation();
        //! @copydoc ContentLocation
        [[nodiscard]] NHeaders::ValueProxy<true, std::string> ContentLocation() const;

        //! @brief Date and time at which the message was originated.
        //! @par Example
        //! @code{.cpp}
        //! auto date{ headers.Date().GetAsOpt() };
        //! @endcode
        NHeaders::ValueProxy<false, std::chrono::utc_clock::time_point> Date();
        //! @copydoc Date
        [[nodiscard]] NHeaders::ValueProxy<true, std::chrono::utc_clock::time_point> Date() const;

        //! @brief Options for the current connection.
        //! @par Example
        //! @code{.cpp}
        //! headers.Connection().TrySet("keep-alive");
        //! auto connection{ headers.Connection().GetAsOpt() };
        //! @endcode
        NHeaders::ListProxy<false, std::string> Connection();
        //! @copydoc Connection
        [[nodiscard]] NHeaders::ListProxy<true, std::string> Connection() const;

        //! @brief Used to signal a protocol change (e.g., "websocket").
        //! @par Example
        //! @code{.cpp}
        //! headers.Upgrade().Add(NHeaders::Upgrade{ "websocket", "13" });
        //! @endcode
        NHeaders::ListProxy<false, NHeaders::Upgrade> Upgrade();
        //! @copydoc Upgrade
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::Upgrade> Upgrade() const;

        //! @brief Indicates header fields present in the trailer of a chunked message.
        //! @par Example
        //! @code{.cpp}
        //! headers.Trailer().Set(std::vector<std::string>{ "X-Checksum", "X-Signature" });
        //! @endcode
        NHeaders::ListProxy<false, std::string> Trailer();
        //! @copydoc Trailer
        [[nodiscard]] NHeaders::ListProxy<true, std::string> Trailer() const;

        //! @brief Path taken by the request/response through proxies (free std::string).
        //! @par Example
        //! @code{.cpp}
        //! headers.Via().Add("1.1 proxy.example");
        //! @endcode
        NHeaders::ListProxy<false, std::string> Via();
        //! @copydoc Via
        [[nodiscard]] NHeaders::ListProxy<true, std::string> Via() const;

        //! @brief Parses the Link header.
        //! @par Example
        //! @code{.cpp}
        //! auto next{ NHeaders::Link{ NHeaders::LinkHeader{
        //!     "https://example.test/next" } }.WithParam("rel", "next") };
        //! headers.Link().Add(next);
        //! @endcode
        NHeaders::ListProxy<false, NHeaders::Link> Link();
        //! @copydoc Link
        [[nodiscard]] NHeaders::ListProxy<true, NHeaders::Link> Link() const;

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
    protected:
        MapType m_headers;

        friend struct std::formatter<Headers>;
        friend struct std::formatter<RequestHeaders>;
        friend struct std::formatter<ResponseHeaders>;
    };
}

#include <Thoth/Http/NHeaders/Headers.tpp>
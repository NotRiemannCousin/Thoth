// Http11.hpp
#pragma once
#include <Thoth/Http/_base.hpp>
#include <Thoth/Http/ExchangeError.hpp>
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <expected>
#include <variant>

namespace Thoth::Http::details_ {
    template<class Stream>
        struct ResponseParseStage {
        ResponseHead data{};
        Stream stream;
    };

    template<class Stream, WritableBodyConcept Body>
    struct ParseCompleteStage : ResponseParseStage<Stream> {
        Body body{};
    };

    //! @brief Handles the parsing and formatting of Http/1.1 messages.
    //! @details Reading and writing share the same knowledge about body framing.
    //! Plain Http/1.0 uses the same machinery, but Http/2 and Http/3 have different
    //! semantics and require their own structs.
    struct Http11 {
        //! @brief Parses the Http response line (version, status code and reason).
        template<typename Stream>
        static std::expected<ResponseParseStage<Stream>, ExchangeError> ParseResponseLine(
            ResponseParseStage<Stream> stage);

        //! @brief Parses the Http headers extracted from a stream.
        template<typename Stream>
        static std::expected<ResponseParseStage<Stream>, ExchangeError> ParseHeaders(
            ResponseParseStage<Stream> stage);



        //! @brief Parses the Http message body.
        template<typename Stream, WritableBodyConcept Body>
        static std::expected<ParseCompleteStage<Stream, Body>, ExchangeError> ParseBody(
            ParseCompleteStage<Stream, Body> stage);

        //! @brief Sets "content-length" or "transfer-encoding: chunked" on headers
        //! depending on the body type.
        //! @see ReadableBodyConcept
        template<ReadableBodyConcept Body>
        static void PrepareBodyHeaders(Headers& headers, const Body& body);

        //! @brief Sends the request line and headers over the wire.
        template<WireSocketConcept Socket>
        static std::expected<std::monostate, ExchangeError> SendRequestLineAndHeaders(
            Socket& socket,
            std::string_view method,
            const auto& url,
            VersionEnum version,
            const Headers& headers);

        //! @brief Sends the body over the wire according to its framing type.
        //! @see ReadableBodyConcept
        template<WireSocketConcept Socket, ReadableBodyConcept Body>
        static std::expected<size_t, ExchangeError> SendBody(Socket& socket, const Body& body);

        static constexpr std::string_view k_crlf     { "\r\n" };
        static constexpr std::string_view k_crlfCrlf { "\r\n\r\n" };
        static constexpr std::string_view k_lastChunk{ "0\r\n\r\n" };
    };
}

#include <Thoth/Http/_base/Http11.tpp>
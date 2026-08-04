// Http11.hpp
#pragma once
#include <Thoth/Http/_base.hpp>
#include <Thoth/Http/ExchangeError.hpp>
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <expected>
#include <variant>

namespace Thoth::Http::details_ {
    //! @brief Handles the parsing and formatting of Http/1.1 messages.
    //! @details Reading and writing share the same knowledge about body framing.
    //! Plain Http/1.0 uses the same machinery, but Http/2 and Http/3 have different
    //! semantics and require their own structs.
    struct Http11 {
        //! @brief Parses the Http response line (version, status code and reason).
        template<typename Stream>
        static std::expected<ResponseParseStage<Stream>, ExchangeError> ParseResponseLine(ResponseParseStage<Stream> stage);

        //! @brief Parses the Http Request line.
        template<typename Stream>
        static std::expected<std::pair<std::string, RequestParseStage<Stream>>, ExchangeError> ParseRequestLine(RequestParseStage<Stream> stage); // TODO: IMPLEMENT

        //! @brief Parses the Http headers extracted from a stream.
        template<typename Stream, class Head>
        static std::expected<ParseStage<Stream, Head>, ExchangeError> ParseHeaders(ParseStage<Stream, Head> stage);


        //! @brief Parses the Http message body.
        template<typename Stream, WritableBodyConcept Body, class Head>
        static std::expected<ParseCompleteStage<Stream, Head, Body>, ExchangeError> ParseBody(
            ParseCompleteStage<Stream, Head, Body> stage);

        //! @brief Sets "content-length" or "transfer-encoding: chunked" on headers
        //! depending on the body type.
        //! @see ReadableBodyConcept
        template<ReadableBodyConcept Body>
        static void PrepareBodyHeaders(Headers& headers, const Body& body);

        //! @brief Sends the request/response line and headers over the wire.
        template<MethodConcept Method, class Head, WireSocketConcept Socket>
            requires (std::same_as<Head, RequestHead> || std::same_as<Head, ResponseHead>)
        static std::expected<std::monostate, ExchangeError> SendMessageHead(Socket& socket, const Head& head);

        //! @brief Sends the body over the wire according to its framing type.
        //! @see ReadableBodyConcept
        template<WireSocketConcept Socket, ReadableBodyConcept Body>
        static std::expected<size_t, ExchangeError> SendBody(Socket& socket, const Body& body);
    };
}

#include <Thoth/Http/_base/Http11.tpp>
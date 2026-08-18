// Http1.hpp
#pragma once
#include <Thoth/Http/_base.hpp>
#include <Thoth/ThothError.hpp>
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <expected>
#include <variant>

namespace Thoth::Http::details_ {
    //! @brief Handles the parsing and formatting of Http/1.1 messages.
    //! @details Reading and writing share the same knowledge about body framing.
    //! Plain Http/1.0 uses the same machinery, but Http/2 and Http/3 have different
    //! semantics and require their own structs.
    struct Http1 {
        template<class Method, WritableBodyConcept ResponseBody, class F, class Stream>
            requires ResponseBodyFactoryConcept<F, ResponseBody>
        static std::expected<Response<Method, ResponseBody>, ThothError> BuildResponse(Stream&& stream, F&& bodyFactory);

        //! @brief Parses the Http response line (version, status code and reason).
        template<class Stream>
        static std::expected<ResponseParseStage<Stream>, ThothError> ParseResponseLine(ResponseParseStage<Stream> stage);

        //! @brief Parses the Http Request line.
        template<class Stream>
        static std::expected<std::pair<std::string, RequestParseStage<Stream>>, ThothError> ParseRequestLine(RequestParseStage<Stream> stage); // TODO: IMPLEMENT

        //! @brief Parses the Http headers extracted from a stream.
        template<class Stream, class Head>
        static std::expected<ParseStage<Stream, Head>, ThothError> ParseHeaders(ParseStage<Stream, Head> stage);


        //! @brief Parses the Http message body.
        template<class Stream, WritableBodyConcept Body, class Head>
        static std::expected<ParseCompleteStage<Stream, Head, Body>, ThothError> ParseBody(
            ParseCompleteStage<Stream, Head, Body> stage);

        //! @brief Sets "content-length" or "transfer-encoding: chunked" on headers
        //! depending on the body type.
        //! @see ReadableBodyConcept
        template<ReadableBodyConcept Body>
        static void PrepareBodyHeaders(Headers& headers, const Body& body);

        //! @brief Sends the request/response line and headers over the wire.
        template<MethodConcept Method, class Head, ConnectionConcept Socket>
            requires (std::same_as<Head, RequestHead> || std::same_as<Head, ResponseHead>)
        static std::expected<std::monostate, ThothError> SendMessageHead(
            Socket& socket, const Head& head, typename Socket::SendOptions options = {});

        //! @brief Sends the body over the wire according to its framing type.
        //! @see ReadableBodyConcept
        template<ConnectionConcept Socket, ReadableBodyConcept Body>
        static std::expected<size_t, ThothError> SendBody(
            Socket& socket, const Body& body, typename Socket::SendOptions options = {});
    };
}

#include <Thoth/Http/_base/Http1.tpp>
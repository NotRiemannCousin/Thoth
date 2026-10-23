#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <exec/task.hpp>

#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Request/RequestHead.hpp>
#include <Thoth/Http/_base/BodyIo/_base/_pch.hpp>
#include <Thoth/Http/_base/Transport/Http1.hpp>

namespace Thoth::Http {
    //! @brief Body factory runs before any body byte is consumed.
    template<class F, class Body>
    concept RequestBodyFactoryConcept = BodyFactoryConcept<F, Body, RequestHead>;

    //! @brief Controls which `Host` values the server accepts.
    struct HostPolicy {
        std::vector<std::string>   allowed{ "localhost" };
        std::optional<std::string> fallbackHost{};
        bool                       matchPort{ false };
    };


    struct FramingEmpty{};
    struct FramingSized{ std::uint64_t size; };
    struct FramingStreamed{};

    using BodyFraming = std::variant<FramingEmpty, FramingSized, FramingStreamed>;

    //! @brief One live request handed to the Router, the Server side of an exchange.
    //!
    //! The head is already parsed (a broken head never reaches a Router, the Server answers those itself through
    //! `Http1::WriteErrorResponse`) and the @ref ReadContext is borrowed from the connection, so a handler can still
    //! pull the body from the wire through @ref ReadAs. The exchange never owns the socket, the connection does,
    //! which keeps keep-alive (and a future connection pool) entirely in the Server's hands.
    //!
    //! @tparam Socket The accepted socket type the connection reads from.
    template<CommonSocketConcept Socket>
    struct Exchange {
        //! @brief Raw method text from the request line. Methods are compile-time on the Client, on the Server they
        //! only exist at runtime, the Router is the one giving them meaning (if it cares at all).
        std::string method;
        //! @brief The parsed request head (target, version, headers).
        RequestHead head;
        //! @brief Body framing of the request.
        BodyFraming framing{};
        //! @brief Where the body still comes from, borrowed from the connection that owns the socket.
        ReadContext<Socket>& context;
        //! @brief Flipped by @ref ReadAs. When the Router never reads the body the Server drains it before the next
        //! request on the same connection (chunked leftovers close it instead, draining those is a gamble).
        bool bodyRead{ false };

        //! @brief Reads the request body into a typed body. The factory runs before any body byte is consumed, so a
        //! refused body (factory error) drains nothing. On failure the stream is left mid-body, the Server answers
        //! with the mapped status and closes, the body dies with its own RAII.
        template<IncomingBodyConcept Body, class F>
            requires RequestBodyFactoryConcept<F, Body>
        exec::task<Body> ReadAs(F&& factory);

        //! @brief `ReadAs` with the stock factory (a default constructed body), already assembled into the typed
        //! @ref Request the route callbacks take. The head is copied in, the callback owns its request.
        template<MethodConcept Method, IncomingBodyConcept Body>
        exec::task<Request<Method, Body>> ReadRequest();


        [[nodiscard]] bool HasBody() const noexcept;
        [[nodiscard]] bool Streamed() const noexcept;
    };


    //! @brief Reads one request head and validates it, producing a live @ref Exchange for the Router or the
    //! @ref ThothError that explains the refusal.
    //!
    //! @details The framing is computed here, once, so nobody re-parses headers to reason about the body later.
    //! A refused head is thrown as a plain ThothError carrying the `StatusCodeEnum` to answer with (413 for an
    //! oversized `Content-Length`, 400 for a `Host` outside the policy), which is everything the caller needs to
    //! answer, no intermediate error type between the head and the wire. Anything the transport throws collapses
    //! into the same shape, a `ConnectionErrorEnum` inside means the socket itself died mid-read and there is
    //! nobody left to answer.
    template<CommonSocketConcept Socket>
    exec::task<Exchange<Socket>> ReceiveHead(ReadContext<Socket>& context, const HostPolicy& hosts);
}

#include <Thoth/Http/Server/Definitions.tpp>

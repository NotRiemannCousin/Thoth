#pragma once
#include <Thoth/Http/_base/Transport/_base.hpp>
#include <Thoth/ThothError.hpp>
#include <Thoth/Http/NHeaders/Headers.hpp>
#include <Thoth/Http/Request/RequestHead.hpp>
#include <Thoth/Http/Response/ResponseHead.hpp>
#include <Thoth/Http/Methods/_base/MethodConcept.hpp>
#include <expected>
#include <optional>
#include <utility>

#include <exec/task.hpp>

namespace Thoth::Http::Transport {
    //! @brief The head's own opinion about the connection, the response and the house rules are decided elsewhere.
    //! HTTP/1.1 lives unless it says `close`, HTTP/1.0 dies unless it says `keep-alive`.
    template<class HeadT>
    bool HeadWantsKeepAlive(const HeadT& head);

    using RequestPair = std::pair<std::string, RequestHead>;

    //! @brief Class that makes the HTTP/1.X communication. It transforms "bytes on the wire" into request/response
    //! pieces and back. It's just the act of send/receive data, no connection pooling, method resolution, routing, etc.
    //!
    //! @note Reading and writing share the same knowledge about body framing. This is specific to HTTP/1.0 and 1.1.
    //!
    //!
    //! @anchor http1_read_tree
    //! ## Reading from the wire
    //!
    //! @verbatim
    //! ReadRequest(ctx, factory)          ReadResponse(ctx, factory)
    //!   ├── ReadRequestHead(ctx)           ├── ReadResponseHead(ctx)
    //!   │     └── ReadHeaders(ctx)         │     └── ReadHeaders(ctx)
    //!   └── ReadBody(ctx, head, factory)   └── ReadBody(ctx, head, factory)
    //!         ├── ReadChunkedBody               (or ReadInto for Content-Length)
    //!         │     ├── ReadLine (chunk size)
    //!         │     ├── ReadInto (chunk data, pushed through BodyIo::Write)
    //!         │     ├── ExpectCrlf (post-data)
    //!         │     └── ReadHeaderBlock (trailers)
    //!         └── Fill(ctx)  ← the ONLY place in the whole read side that
    //!                          co_awaits the transfer source
    //! @endverbatim
    //!
    //! Every function receives the same @ref ReadContext (source + ExchangeBuffer + HttpLimits). A Recv may deliver
    //! more bytes than a stage needs, so leftovers flow to the next stage through the shared window.
    //! @ref ReadContext::deadline is the deadline of the current operation.
    //!
    //! Some body types needs a preparation stage before start to receive bytes (like opening files or asserting that
    //! some headers are specified). So the body factory runs *before* the first body byte is consumed and the Server
    //! can reject a body (factory returning an error) without even draining the stream.
    //!
    //! ## Writing to the wire
    //!
    //! @verbatim
    //! WriteRequest(socket, request)            WriteResponse(socket, response)
    //!   ├── PrepareBodyHeaders (pure)            ├── PrepareBodyHeaders (pure)
    //!   ├── WriteRequestHead<S, Method>          ├── WriteResponseHead
    //!   │     └── Send (line + headers in        │     └── Send
    //!   │         ONE send, formatter-rendered)
    //!   └── WriteBody (BodyIo::Read slices,      └── WriteBody
    //!       Content-Length or chunked frames + last-chunk)
    //! @endverbatim
    //!
    //! ## Sync x Async
    //!
    //! This transport engine is designed to work with every `{ Sync, Async } x { Client, Server }` socket with a
    //! single interface. Joining sync and async requires both paths to use corroutines because of `Send/Recv` calls,
    //! but the sync one is executed sincronously, the convergence is just to align the signature and implementation.
    //!
    //! @note One minute of silence to the bealty Sync HTTP parser that used `RecvStream()` (/joke).
    //!
    //! ## Error model
    //!
    //! Becuase of how senders/async C++ works, errors are thrown, not returned. A single try/catch at the Client/Server
    //! boundary will convert them back into the `expected<>` world in sync sockets and given that `exec::task` maps the
    //! error channel to `std::exception_ptr`, you can remap it back to `ThothError` using `let_error`. (it's sad but is
    //! the price to pay for a single simple implementation):
    //!
    //! @code{.cpp}
    //! try { co_return co_await details_::Http1::ReadResponse(ctx, factory); }
    //! catch (const Hermes::TransferError& e) { return std::unexpected{ ThothError{ e.error } }; }
    //! catch (const ThothError& e)            { return std::unexpected{ e }; }
    //! @endcode
    //!
    //! | Failure                                          | Thrown as                                                          |
    //! |--------------------------------------------------|--------------------------------------------------------------------|
    //! | Peer closed mid-line / mid-body                  | `ThothError{ConnectionErrorEnum::ConnectionClosed}`                |
    //! | Sync transport error                             | `ThothError{ConnectionErrorEnum}` (from SyncAsAsync)               |
    //! | Async transport error                            | `Hermes::TransferError` (from the co_await-ed sender)              |
    //! | deadline hitted during send                      | `ThothError{ConnectionErrorEnum::SendTimeout}` (socket options)    |
    //! | deadline hitted before the start line            | `ThothError{MessageParseErrorEnum::InvalidStartLine}`              |
    //! | deadline hitted mid-headers / mid-body           | `ThothError{ConnectionErrorEnum::ConnectionClosed}`                |
    //! | Malformed start line / chunk frame               | `ThothError{MessageParseErrorEnum::InvalidStartLine}`              |
    //! | Invalid version                                  | `ThothError{MessageParseErrorEnum::InvalidVersion}`                |
    //! | Bad headers / CL and TE together / TE != chunked | `ThothError{MessageParseErrorEnum::InvalidHeaders}`                |
    //! | Line/head/body over HttpLimits                   | `HeadersTooLarge` / `InvalidStartLine` / `InvalidHeaders`          |
    //! | Chunked body on HTTP/1.0                         | `ThothError{MessageParseErrorEnum::VersionNeedsContentLength}`     |
    //! | Body push refused (ok with zero bytes consumed)  | `ThothError{GenericError{...}}`                                    |
    //! | Body factory refused                             | the factory's `ThothError`                                         |
    //!
    //! ## Framing rules
    //!
    //! As specified by the RFC, while reading `Transfer-Encoding: chunked` have higher priority. If absent,
    //! `Content-Length` is setted with size 0 (so responses like 204 parse fine) (chunked on HTTP/1.0 is rejected).
    //! On the write side the framing comes from `BodyIo::Size` (a known size uses `Content-Length`, `nullopt` uses
    //! `Transfer-Encoding: chunked`). On HTTP/1.0 an unsized outgoing body cannot be chunked, so the caller has to
    //! handle that itself (buffer it up to a known size or refuse the exchange).
    //!
    //! ## Request and Method handling
    //!
    //! @ref ReadRequestHead returns the raw method as `std::string` because methods are typed (`MethodConcept`) and
    //! cannot be resolved while reading the wire. Using `pair<string, RequestHead>` instead of a typed `Response` is a
    //! sad decision but I can live with that. Resolving the method is the Client/Server's job (the Server will forward
    //! this decision to the `Router`).
    //!
    //! @see Thoth::Http::Client.
    //! @see Thoth::Http::Server.
    struct Http1 {
#pragma region Read family

        //! @brief Request line + headers. See @ref http1_read_tree.
        template<CommonSocketConcept S>
        static exec::task<RequestPair> ReadRequestHead(ReadContext<S>& context);

        //! @brief Status line + headers. See @ref http1_read_tree.
        template<CommonSocketConcept S>
        static exec::task<ResponseHead> ReadResponseHead(ReadContext<S>& context);



        //! @brief Head + body of a request. Method text comes back as a raw string (see class doc).
        template<CommonSocketConcept S, IncomingBodyConcept Body, class F>
            requires BodyFactoryConcept<F, Body, RequestHead>
        static exec::task<std::pair<RequestPair, Body>> ReadRequest(ReadContext<S>& context, F&& bodyFactory);

        //! @copydoc ReadRequest
        template<CommonSocketConcept S, IncomingBodyConcept Body>
            requires std::default_initializable<Body>
        static exec::task<std::pair<RequestPair, Body>> ReadRequest(ReadContext<S>& context);

        //! @brief Head + body of a response. The client assemblies `Response<Method, Body>` from the
        //! `pair<ResponseHead, Body>` returned.
        template<CommonSocketConcept S, IncomingBodyConcept Body, class F>
            requires BodyFactoryConcept<F, Body, ResponseHead>
        static exec::task<std::pair<ResponseHead, Body>> ReadResponse(ReadContext<S>& context, F&& bodyFactory);

        template<CommonSocketConcept S, IncomingBodyConcept Body>
            requires std::default_initializable<Body>
        static exec::task<std::pair<ResponseHead, Body>> ReadResponse(ReadContext<S>& context);



        //! @brief Reads the message body according to the framing declared by the head.
        //! @details Used by both sides. `bodyFactory` Runs before any body byte is consumed, on error nothing is
        //! drained and the error propagates. The incoming body is blind to the wire framing, it only sees a stream of
        //! windows pushed through `BodyIo::Write` and one `BodyIo::FinishWrite` at the end (only on success).
        template<CommonSocketConcept S, IncomingBodyConcept Body, class HeadT, class F>
            requires BodyFactoryConcept<F, Body, HeadT>
        static exec::task<Body> ReadBody(ReadContext<S>& context, const HeadT& head, F&& bodyFactory);

        //! @copydoc ReadBody
        template<CommonSocketConcept S, IncomingBodyConcept Body, class HeadT>
            requires std::default_initializable<Body>
        static exec::task<Body> ReadBody(ReadContext<S>& context, const HeadT& head);

#pragma endregion

#pragma region Write family

        //! @brief Formats request line + headers (via `std::formatter<RequestHead>`) and sends everything in a single
        //! send call.
        template<CommonSocketConcept S, MethodConcept Method>
        static exec::task<void> WriteRequestHead(S& socket, const RequestHead& head,
            std::optional<ExchangeDeadline> deadline = std::nullopt);

        //! @brief Formats status line + headers (via `std::formatter<ResponseHead>`) and sends everything in a single
        //! send call.
        template<CommonSocketConcept S>
        static exec::task<void> WriteResponseHead(S& socket, const ResponseHead& head,
            std::optional<ExchangeDeadline> deadline = std::nullopt);



        //! @brief `PrepareBodyHeaders` + head + body, in order. Takes the message by non-const reference because the
        //! framing headers (`Content-Length` or `Transfer-Encoding: chunked`) are stamped into it. Pass a copy if the
        //! original must stay untouched.
        //! If the Server needs custom framing control, call the three pieces directly instead.
        template<CommonSocketConcept S, MethodConcept Method, OutgoingBodyConcept Body>
        static exec::task<void> WriteRequest(S& socket, Request<Method, Body>& request,
            std::optional<ExchangeDeadline> deadline = std::nullopt);

        //! @brief `PrepareBodyHeaders` + head + body, in order. Takes the message by non-const reference because the
        //! framing headers (`Content-Length` or `Transfer-Encoding: chunked`) are stamped into it. Pass a copy if the
        //! original must stay untouched.
        //! If the Server needs custom framing control, call the three pieces directly instead.
        template<CommonSocketConcept S, MethodConcept Method, OutgoingBodyConcept Body>
        static exec::task<void> WriteResponse(S& socket, Response<Method, Body>& response,
            std::optional<ExchangeDeadline> deadline = std::nullopt);



        //! @brief Sends the body according to its `Size` framing, pulling slices through `BodyIo::Read`. Takes the
        //! body by non-const reference because lazy bodies (input ranges) are consumed. Empty slices are skipped
        //! defensively (a zero sized chunk on the wire is the last-chunk marker).
        //! @return Total body bytes written (framing bytes excluded).
        template<CommonSocketConcept S, OutgoingBodyConcept Body>
        static exec::task<std::size_t> WriteBody(S& socket, Body& body,
            std::optional<ExchangeDeadline> deadline = std::nullopt);

#pragma endregion

#pragma region Pure helpers (kept from the sync era)

        //! @brief Sets `content-length` or `transfer-encoding: chunked` on headers depending on what `BodyIo::Size`
        //! reports. Must run before the head is rendered.
        //! @see OutgoingBodyConcept.
        template<OutgoingBodyConcept Body>
        static void PrepareBodyHeaders(Headers& headers, const Body& body);

        static std::expected<std::monostate, ThothError> ValidateFraming(const Headers& headers);

#pragma endregion
    };
}

#include <Thoth/Http/_base/Transport/Http1.tpp>

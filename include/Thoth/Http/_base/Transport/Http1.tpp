#pragma once
#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Utils/Await.hpp>
#include <Thoth/Utils/Scanner.hpp>

#include <cctype>
#include <cstring>
#include <format>
#include <string>
#include <string_view>

#include <exec/when_any.hpp>

namespace Thoth::Http::Transport {
    namespace details_ {
        static constexpr std::string_view k_crlf     { "\r\n" };
        static constexpr std::string_view k_crlfCrlf { "\r\n\r\n" };
        static constexpr std::string_view k_lastChunk{ "0\r\n\r\n" };

        //! @brief Send honoring the exchange deadline when the transport supports it.
        template<CommonSocketConcept S, class T>
        auto Send(S& socket, T&& data, const std::optional<ExchangeDeadline>& deadline) {
            if constexpr (SyncSocketConcept<S>) {
                typename S::SendOptions options{ .deadline = deadline };
                return socket.Send(std::forward<T>(data), std::move(options));
            } else {
                auto sendSender{ socket.Send(std::forward<T>(data)) };
                auto sched{ stdexec::get_completion_scheduler<stdexec::set_value_t>(
                            stdexec::get_env(sendSender)) };
                return exec::when_any(
                    std::move(sendSender),
                    exec::schedule_at(sched, deadline.value_or(ExchangeDeadline::max()))
                            | stdexec::let_value([] {
                                return stdexec::just_error(ConnectionErrorEnum::SendTimeout);
                            })
                );
            }
        }

        //! @brief Recv honoring the exchange deadline when the transport supports it.
        template<CommonSocketConcept S>
        auto Recv(S& socket, std::span<char> buffer, const Hermes::RecvModeEnum mode,
                          const std::optional<ExchangeDeadline>& deadline) {
            if constexpr (SyncSocketConcept<S>) {
                typename S::RecvOptions options{ .deadline = deadline };
                return socket.Recv(std::move(buffer), mode, std::move(options));
            } else {
                auto recvSender{ socket.Recv(buffer, mode) };
                auto sched{ stdexec::get_completion_scheduler<stdexec::set_value_t>(
                            stdexec::get_env(recvSender)) };
                return exec::when_any(
                    std::move(recvSender),
                    exec::schedule_at(sched, deadline.value_or(ExchangeDeadline::max()))
                            | stdexec::let_value([] {
                                return stdexec::just_error(ConnectionErrorEnum::ReceiveTimeout);
                            })
                );
            }
        }
    }

#pragma region keep-alive helpers

    template<class HeadT>
    bool HeadWantsKeepAlive(const HeadT& head) {
        bool askedKeepAlive{ false };
        bool askedClose{ false };

        if (const auto connection{ head.headers.Connection().GetAsOpt() })
            for (const auto& token : *connection) {
                askedKeepAlive = askedKeepAlive || String::TokenEquals(token, "keep-alive");
                askedClose     = askedClose || String::TokenEquals(token, "close");
            }

        if (head.version == VersionEnum::Http1_0)
            return askedKeepAlive && !askedClose;

        return !askedClose;
    }

#pragma endregion



#pragma region Read-side primitives

    //! @brief Brings one batch of bytes from the source into the ExchangeBuffer (the only place on the read side that
    //! co_awaits the source).
    template<CommonSocketConcept S>
    exec::task<std::size_t> Fill(ReadContext<S>& context) {
        const auto freeSpace{ context.buffer.Free() };
        if (freeSpace.empty())
            co_return std::size_t{ 0 };

        const auto bytes{ co_await Utils::Await(
            details_::Recv(
                context.source,
                std::span<char>{ reinterpret_cast<char*>(freeSpace.data()), freeSpace.size() },
                Hermes::RecvModeEnum::Any,
                context.deadline)
        ) };

        context.buffer.Commit(bytes);
        co_return bytes;
    }

    //! @brief Reads one CRLF-terminated line (CRLF consumed, not returned). A trailing CR is left in the window, as
    //! it may be the first half of a CRLF split by a Recv boundary and the next scan pairs it with the incoming LF.
    template<CommonSocketConcept S>
    exec::task<std::string> ReadLine(ReadContext<S>& context, const std::size_t maxBytes) {
        std::string line{};

        for (;;) {
            const auto window{ context.buffer.Available() };

            if (!window.empty()) {
                const std::string_view view{
                    reinterpret_cast<const char*>(window.data()), window.size() };
                if (const auto pos{ view.find(details_::k_crlf) }; pos != std::string_view::npos) {
                    line.append(view.substr(0, pos));
                    context.buffer.Consume(pos + details_::k_crlf.size());

                    if (line.size() > maxBytes)
                        throw ThothError{ MessageParseErrorEnum::InvalidStartLine };

                    co_return line;
                }

                if (view.back() == '\r') {
                    line.append(view.substr(0, view.size() - 1));
                    context.buffer.Consume(window.size() - 1);
                } else {
                    line.append(view);
                    context.buffer.Consume(window.size());
                }

                if (line.size() > maxBytes)
                    throw ThothError{ MessageParseErrorEnum::InvalidStartLine };
            }

            if (context.buffer.Free().empty())
                throw ThothError{ MessageParseErrorEnum::InvalidStartLine }; // single line > window (16 KiB)

            if (co_await Fill(context) == 0)
                throw ThothError{ ConnectionErrorEnum::ConnectionClosed };
        }
    }

    //! @brief Reads the header block (or the chunked-trailer block) line by line, accumulating it as "name: value\r\n...".
    template<CommonSocketConcept S>
    exec::task<std::string> ReadHeaderBlock(ReadContext<S>& context) {
        std::string block{};

        for (;;) {
            auto line{ co_await ReadLine(context, context.limits.maxHeadBytes) };
            if (line.empty())
                co_return block;

            if (block.size() + line.size() + details_::k_crlf.size() > context.limits.maxHeadBytes)
                throw ThothError{ MessageParseErrorEnum::HeadersTooLarge };

            block += line;
            block += details_::k_crlf;
        }
    }

    //! @brief Consumes exactly one CRLF (the frame separator after each chunk's data).
    template<CommonSocketConcept S>
    exec::task<void> ExpectCrlf(ReadContext<S>& context) {
        std::array<char, 2> acc{};
        std::size_t got{};

        while (got < acc.size()) {
            const auto window{ context.buffer.Available() };

            if (!window.empty()) {
                const auto take{ std::min<std::size_t>(window.size(), acc.size() - got) };
                std::memcpy(acc.data() + got, window.data(), take);
                context.buffer.Consume(take);
                got += take;
                continue;
            }

            if (co_await Fill(context) == 0)
                throw ThothError{ ConnectionErrorEnum::ConnectionClosed };
        }

        if (std::string_view{ acc.data(), acc.size() } != details_::k_crlf)
            throw ThothError{ MessageParseErrorEnum::InvalidStartLine };
    }

    //! @brief Pushes one window worth of bytes into the body through @ref BodyIo::Write, honoring the partial-write
    //! contract (every call consumes at least one byte or fails, ok with zero consumed kills the exchange instead of
    //! spinning on it). The bytes are already in the window, no Recv happens inside the loop.
    template<IncomingBodyConcept Body>
    exec::task<void> PushWindow(BodyIoFor<Body>& io, Body& body, std::span<std::byte> window) {
        std::size_t taken{};

        while (taken < window.size()) {
            const auto [consumed, res]{ co_await Utils::Await(io.Write(body, window.subspan(taken))) };
            if (!res)
                throw res.error();
            if (consumed == 0)
                throw ThothError{ GenericError{ "BodyIo::Write consumed zero bytes without an error" } };

            taken += consumed;
        }
    }

    //! @brief Appends @p remaining bytes to the body, refilling through @ref Fill. The body is blind to the wire
    //! framing, it only sees a stream of windows pushed through @ref BodyIo::Write.
    template<CommonSocketConcept S, IncomingBodyConcept Body>
    exec::task<void> ReadInto(ReadContext<S>& context, BodyIoFor<Body>& io, Body& body, std::uint64_t remaining) {
        while (remaining > 0) {
            const auto window{ context.buffer.Available() };

            if (window.empty()) {
                if (co_await Fill(context) == 0)
                    throw ThothError{ ConnectionErrorEnum::ConnectionClosed };
                continue;
            }

            const auto take{ std::min<std::uint64_t>(window.size(), remaining) };
            co_await PushWindow(io, body, window.first(static_cast<std::size_t>(take)));
            context.buffer.Consume(static_cast<std::size_t>(take));
            remaining -= take;
        }
    }

    //! @brief `HTTP/1.x` -> VersionEnum.
    constexpr std::expected<VersionEnum, MessageParseErrorEnum>
    ParseHttpVersion(const std::string_view version) {
        if (version.size() != 8 || !version.starts_with("HTTP/1."))
            return std::unexpected{ MessageParseErrorEnum::InvalidStartLine };

        switch (version[7]) {
            case '0':  return VersionEnum::Http1_0;
            case '1':  return VersionEnum::Http1_1;
            default:   return std::unexpected{ MessageParseErrorEnum::InvalidVersion };
        }
    }

    //! @brief Parses the block into the head's typed headers and validates the
    //! framing declarations (CL and TE are mutually exclusive; TE must be chunked).
    template<CommonSocketConcept S, class HeadersT>
    exec::task<HeadersT> ReadHeaders(ReadContext<S>& context) {
        const auto block{ co_await ReadHeaderBlock(context) };

        auto headersRes{ HeadersT::Parse(block, context.limits.maxHeadBytes) };
        if (!headersRes)
            throw ThothError{ headersRes.error() };

        if (const auto framingRes{ Http1::ValidateFraming(*headersRes) }; !framingRes)
            throw framingRes.error();

        co_return std::move(*headersRes);
    }

    //! @brief Chunked body pump the chunk state (running total, current size, trailer phase) lives in these coroutine
    //! locals between suspensions. The body only sees windows, the chunk boundaries die here.
    template<CommonSocketConcept S, IncomingBodyConcept Body>
    exec::task<void> ReadChunkedBody(ReadContext<S>& context, BodyIoFor<Body>& io, Body& body) {
        std::uint64_t totalBodySize{};

        for (;;) {
            constexpr std::size_t k_maxChunkLineLength{ 64 };
            const auto sizeLine{ co_await ReadLine(context, k_maxChunkLineLength) };

            const auto chunkLength{ Utils::Scan<std::size_t>(sizeLine, "x") };
            if (!chunkLength)
                throw ThothError{ MessageParseErrorEnum::InvalidStartLine };

            if (totalBodySize + *chunkLength > context.limits.maxBodyBytes)
                throw ThothError{ MessageParseErrorEnum::InvalidHeaders };
            totalBodySize += *chunkLength;

            if (*chunkLength == 0) {
                co_await ReadHeaderBlock(context); // trailers (possibly empty) + final CRLF
                co_return;
            }

            co_await ReadInto(context, io, body, *chunkLength);
            co_await ExpectCrlf(context);
        }
    }

#pragma endregion

#pragma region Http1::Read*

    template<CommonSocketConcept S>
    exec::task<RequestPair> Http1::ReadRequestHead(ReadContext<S>& context) {
        constexpr std::string_view k_placeholderAuthority{ "thoth.invalid" };

        const auto line{ co_await ReadLine(context, context.limits.maxRequestLineBytes) };

        const auto firstSpace{ line.find(' ') };
        const auto secondSpace{
            firstSpace == std::string::npos ? std::string::npos : line.find(' ', firstSpace + 1) };

        if (firstSpace == std::string::npos || secondSpace == std::string::npos
            || firstSpace == 0 || secondSpace == firstSpace + 1)
            throw ThothError{ MessageParseErrorEnum::InvalidStartLine };

        const std::string_view method{ line.data(), firstSpace };
        const std::string_view target{
            line.data() + firstSpace + 1, secondSpace - firstSpace - 1 };
        const std::string_view version{
            line.data() + secondSpace + 1, line.size() - secondSpace - 1 };

        const auto versionRes{ ParseHttpVersion(version) };
        if (!versionRes) throw ThothError{ versionRes.error() };

        RequestHeaders headers{ co_await ReadHeaders<S, RequestHeaders>(context) };

        const auto host{ headers.Host().Get() };
        auto url{ target.front() == '/'
            ? Url::FromOriginForm(host ? std::string_view{ *host } : k_placeholderAuthority, target)
            : Url::FromUrl(std::string{ target }) };
        if (!url) throw url.error();

        co_return { std::string{ method }, RequestHead{ .url = std::move(*url), .version = *versionRes,
                                                        .headers = std::move(headers) } };
    }

    template<CommonSocketConcept S>
    exec::task<ResponseHead> Http1::ReadResponseHead(ReadContext<S>& context) {
        const auto line{ co_await ReadLine(context, context.limits.maxRequestLineBytes) };

        // "HTTP/1.x SP ddd SP reason" - the space after the status is mandatory
        // (empty reasons still carry the space), same strictness the sync parser had.
        if (line.size() < 13 || line[12] != ' ')
            throw ThothError{ MessageParseErrorEnum::InvalidStartLine };

        auto head{ ResponseHead{} };

        const auto versionRes{ ParseHttpVersion(std::string_view{ line }.substr(0, 8)) };
        if (!versionRes)
            throw ThothError{ versionRes.error() };
        head.version = *versionRes;

        if (line[8] != ' '
            || !std::isdigit(static_cast<unsigned char>(line[9]))
            || !std::isdigit(static_cast<unsigned char>(line[10]))
            || !std::isdigit(static_cast<unsigned char>(line[11])))
            throw ThothError{ MessageParseErrorEnum::InvalidStartLine };

        head.status = static_cast<StatusCodeEnum>(
            (line[9] - '0') * 100 + (line[10] - '0') * 10 + (line[11] - '0'));

        head.headers = co_await ReadHeaders<S, decltype(head.headers)>(context);

        co_return head;
    }



    template<CommonSocketConcept S, IncomingBodyConcept Body, class F>
        requires BodyFactoryConcept<F, Body, RequestHead>
    exec::task<std::pair<RequestPair, Body>> Http1::ReadRequest(ReadContext<S>& context, F&& bodyFactory) {
        auto headRes{ co_await ReadRequestHead(context) };
        auto body{ co_await ReadBody<S, Body>(context, headRes.second, std::forward<F>(bodyFactory)) };
        co_return { std::move(headRes), std::move(body) };
    }

    template<CommonSocketConcept S, IncomingBodyConcept Body>
        requires std::default_initializable<Body>
    exec::task<std::pair<RequestPair, Body>> Http1::ReadRequest(ReadContext<S>& context) {
        return ReadRequest<S, Body>(context, [](const RequestHead&) { return std::expected<Body, ThothError>{}; });
    }


    template<CommonSocketConcept S, IncomingBodyConcept Body, class F>
        requires BodyFactoryConcept<F, Body, ResponseHead>
    exec::task<std::pair<ResponseHead, Body>> Http1::ReadResponse(ReadContext<S>& context, F&& bodyFactory) {
        auto head{ co_await ReadResponseHead(context) };
        auto body{ co_await ReadBody<S, Body>(context, head, std::forward<F>(bodyFactory)) };
        co_return { std::move(head), std::move(body) };
    }

    template<CommonSocketConcept S, IncomingBodyConcept Body>
        requires std::default_initializable<Body>
    exec::task<std::pair<ResponseHead, Body>> Http1::ReadResponse(ReadContext<S>& context) {
        return ReadResponse<S, Body>(context, [](const ResponseHead&) { return std::expected<Body, ThothError>{}; });
    }


    template<CommonSocketConcept S, IncomingBodyConcept Body, class HeadT, class F>
        requires BodyFactoryConcept<F, Body, HeadT>
    exec::task<Body> Http1::ReadBody(ReadContext<S>& context, const HeadT& head, F&& bodyFactory) {
        auto bodyExp{ std::invoke(bodyFactory, head) }; // before any body byte (doc)
        if (!bodyExp)
            throw ThothError{ bodyExp.error() };

        Body&        body{ *bodyExp };
        BodyIoFor<Body> io{};

        // Framing: TE: chunked wins; otherwise Content-Length (absent = 0).
        const auto transferEncoding{ head.headers.TransferEncoding().Get() };
        const auto isChunked{
            transferEncoding && transferEncoding->size() == 1
                && transferEncoding->front() == NHeaders::TransferEncodingEnum::Chunked };

        if (isChunked) {
            if (head.version == VersionEnum::Http1_0)
                throw ThothError{ MessageParseErrorEnum::VersionNeedsContentLength };

            co_await ReadChunkedBody(context, io, body);
        } else {
            std::uint64_t contentLength{ 0 };
            if (const auto lengthRes{ head.headers.ContentLength().GetWithDefault(std::uint64_t{ 0 }) }; lengthRes)
                contentLength = *lengthRes;

            if (contentLength > context.limits.maxBodyBytes)
                throw ThothError{ MessageParseErrorEnum::InvalidHeaders };

            co_await ReadInto(context, io, body, contentLength);
        }

        // commit/flush of the incoming side (compressor, FILE*), only on success, failure paths leave the body
        // to its RAII destructor without flushing.
        co_await Utils::Await(io.FinishWrite(body));

        co_return std::move(body);
    }


    template<CommonSocketConcept S, IncomingBodyConcept Body, class HeadT>
        requires std::default_initializable<Body>
    exec::task<Body> Http1::ReadBody(ReadContext<S>& context, const HeadT& head) {
        return ReadBody<S, Body, HeadT>(context, head, [](const HeadT&) { return std::expected<Body, ThothError>{}; });
    }

#pragma endregion

#pragma region Http1::Write*

    template<CommonSocketConcept S, MethodConcept Method>
    exec::task<void>
    Http1::WriteRequestHead(S& socket, const RequestHead& head, std::optional<ExchangeDeadline> deadline) {
        co_await Utils::Await(details_::Send(socket, std::format("{} {}", Method::MethodName(), head), deadline));
    }

    template<CommonSocketConcept S>
    exec::task<void> Http1::WriteResponseHead(S& socket, const ResponseHead& head, std::optional<ExchangeDeadline> deadline) {
        co_await Utils::Await(details_::Send(socket, std::format("{}", head), deadline));
    }

    template<CommonSocketConcept S, OutgoingBodyConcept Body>
    exec::task<std::size_t> Http1::WriteBody(S& socket, Body& body, std::optional<ExchangeDeadline> deadline) {
        BodyIoFor<Body> io{};
        const auto   size{ io.Size(body) };
        std::size_t  totalBytes{};

        for (;;) {
            const std::optional<ByteView> chunk{ co_await Utils::Await(io.Read(body)) };
            if (!chunk) break; // nullopt, body drained

            if (chunk->empty()) continue; // defensive, an empty slice mid-body is a broken body (see BodyIo doc)

            if (!size)
                co_await Utils::Await(details_::Send(
                    socket, std::format("{:x}{}", chunk->size(), details_::k_crlf), deadline));

            co_await Utils::Await(
                details_::Send(socket,
                    std::string_view{reinterpret_cast<const char*>(chunk->data()), chunk->size() }, deadline)
            );

            if (!size)
                co_await Utils::Await(details_::Send(socket, details_::k_crlf, deadline));

            totalBytes += chunk->size();
        }

        if (!size)
            co_await Utils::Await(details_::Send(socket, details_::k_lastChunk, deadline));

        co_return totalBytes;
    }

    template<CommonSocketConcept S, MethodConcept Method, OutgoingBodyConcept Body>
    exec::task<void>
    Http1::WriteRequest(S& socket, Request<Method, Body>& request, std::optional<ExchangeDeadline> deadline) {
        PrepareBodyHeaders(request.headers, request.body);
        co_await WriteRequestHead<S, Method>(socket, request, deadline);
        co_await WriteBody(socket, request.body, deadline);
    }

    template<CommonSocketConcept S, MethodConcept Method, OutgoingBodyConcept Body>
    exec::task<void>
    Http1::WriteResponse(S& socket, Response<Method, Body>& response, std::optional<ExchangeDeadline> deadline) {
        PrepareBodyHeaders(response.headers, response.body);
        co_await WriteResponseHead(socket, response, deadline);
        co_await WriteBody(socket, response.body, deadline);
    }

#pragma endregion

#pragma region Pure helpers

    template<OutgoingBodyConcept Body>
    void Http1::PrepareBodyHeaders(Headers& headers, const Body& body) {
        headers.Remove("transfer-encoding");
        headers.Remove("content-length");

        // one path for every body, the optional is what decides the framing (a known size also covers zero, which
        // is a valid Content-Length: 0 and never means "unknown").
        if (const auto size{ BodyIoFor<Body>{}.Size(body) })
            headers.ContentLength().Set(*size);
        else
            headers.TransferEncoding().Add(NHeaders::TransferEncodingEnum::Chunked);
    }

    inline std::expected<std::monostate, ThothError> Http1::ValidateFraming(const Headers& headers) {
        if (headers.Exists("content-length") && headers.Exists("transfer-encoding"))
            return ThothUnex{ MessageParseErrorEnum::InvalidHeaders };

        if (headers.Exists("transfer-encoding")) {
            const auto transferEncoding{ headers.TransferEncoding().Get() };
            if (!transferEncoding || transferEncoding->size() != 1
                || transferEncoding->front() != NHeaders::TransferEncodingEnum::Chunked)
                return ThothUnex{ MessageParseErrorEnum::InvalidHeaders };
        }

        return std::monostate{};
    }

#pragma endregion

}
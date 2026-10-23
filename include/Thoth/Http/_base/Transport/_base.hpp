#pragma once
#include <chrono>
#include <array>
#include <optional>
#include <span>

#include <Hermes/Socket/Sync/ClientSocket.hpp>
#include <Hermes/Socket/Sync/ServerSocket.hpp>
#include <Hermes/Socket/Async/AsyncClientSocket.hpp>
#include <Hermes/Socket/Async/AsyncServerSocket.hpp>


#include <Thoth/Http/_base/BodyIo/_base/_pch.hpp>
#include <Thoth/Http/Methods/_base/MethodConcept.hpp>
#include <exec/timed_scheduler.hpp>

namespace Thoth::Http {

    template<MethodConcept Method, BodyConcept Body>
    struct Request;

    template<MethodConcept Method, BodyConcept Body>
    struct Response;

#pragma region Limits

    //! @brief Per-exchange ceilings and deadlines shared by @ref Client and @ref Server, independent of the HTTP
    //! flavour (1.x today, 2/3 reuse the same struct).
    //!
    //! Http1 itself consumes @ref maxBodyBytes, @ref maxRequestLineBytes and @ref maxHeadBytes directly, the remaining
    //! fields (connection ceilings and the Milliseconds deadlines) are consumed by the Client/Server connection
    //! managers, which translate them into socket Send options and into @ref ReadContext::deadline.
    struct HttpLimits {
        using Milliseconds = std::chrono::duration<uint16_t, std::milli>;

        //! @brief Caps the total body size accepted, whether framed by Content-Length
        //! or by the running sum of all chunks (a peer can't dodge it by splitting
        //! the body into many small chunks).
        std::uint64_t maxBodyBytes{ 0x14000000 };
        //! @brief Caps a single request/status line, CRLF excluded.
        std::uint32_t maxRequestLineBytes{ 8 * 1024 };
        //! @brief Caps the whole header block (and each individual header line).
        std::uint32_t maxHeadBytes{ 64 * 1024 };
        //! @brief Header count ceiling. Enforced by the Server when matching the Router
        //! (Headers::Parse bounds bytes, not entries).
        std::uint32_t maxHeaderCount{ 100 };

        std::uint16_t maxConnections{ 1024 };
        std::uint16_t maxRequestsPerConnection{ 1000 };

        Milliseconds  headerTimeout{ 30'000 };
        Milliseconds  bodyTimeout{ 60'000 };
        Milliseconds  keepAliveIdle{ 60'000 };
    };

#pragma endregion

#pragma region ExchangeBuffer

    //! @brief Fixed-size sliding window that accumulates intermediate Recvs between parse steps (head lines, chunk
    //! frames, body slices).
    //!
    //! The storage is a lazily allocated `unique_ptr` (nullptr until the first Recv), so exchanges that never read a
    //! byte pay nothing (which is also what makes it pool-friendly, a Server can hand the whole struct back to a
    //! connection pool instead of freeing it).
    //!
    //! @note Members are intentionally public, the Server trio and the connection pools poke at them directly.
    struct ExchangeBuffer {
        //! @brief 16 KiB window. Every partial line is spilled into the parser's own
        //! accumulator before a refill (see details_::ReadLine), so a line longer than
        //! the window still parses as long as it respects the HttpLimits caps.
        static constexpr std::size_t k_capacity{ 0x4000 };

        std::unique_ptr<std::array<std::byte, k_capacity>> buffer{};
        std::uint16_t readIdx{};    //!< First byte not consumed by the parser yet.
        std::uint16_t filledIdx{};  //!< One past the last byte brought by a Recv.

        //! @brief Bytes brought from the wire and not consumed yet (the parse window).
        [[nodiscard]] std::span<std::byte> Available() const noexcept;

        //! @brief Writable area for the next Recv. Allocates the storage on first use.
        [[nodiscard]] std::span<std::byte> Free() noexcept;

        //! @brief Reports that a Recv brought @p bytes bytes into Free().
        void Commit(std::size_t bytes) noexcept;

        //! @brief Reports that the parser consumed @p bytes bytes. When the window
        //! drains completely it resets to zero, so the next Recv reuses the whole
        //! buffer without any compaction cost.
        void Consume(std::size_t bytes) noexcept;
    };

#pragma endregion

#pragma region Shared read context

    namespace details_ {
        using StoppableEnv = decltype(stdexec::prop(
            stdexec::get_stop_token,
            std::declval<stdexec::inplace_stop_token>()
        ));

        template<class Sender>
        concept StoppableSender = stdexec::sender<Sender>
            && stdexec::sends_stopped<Sender, StoppableEnv>;
    }

    //! @brief Used to represent deadlines in this lib.
    using ExchangeDeadline = Hermes::DeadlineTimepoint;

    template<class T>
    concept SyncSocketConcept = (Hermes::ServerSocketConcept<T> || Hermes::ClientSocketConcept<T>)
            && requires(typename T::SendOptions& sendOpts, typename T::RecvOptions& recvOpts,
                             const std::optional<ExchangeDeadline> deadline) {
            { sendOpts.deadline = deadline };
            { recvOpts.deadline = deadline };
        };

    template<class T>
    concept AsyncSocketConcept = (Hermes::AsyncClientSocketConcept<T> || Hermes::AsyncServerSocketConcept<T>)
        && requires(T& sock, std::span<std::byte> rx, std::span<const std::byte> tx) {
            { sock.Send(tx) } -> details_::StoppableSender;
            { sock.Recv(rx) } -> details_::StoppableSender;

            { stdexec::get_completion_scheduler<stdexec::set_value_t>(stdexec::get_env(sock.Send(tx))) } -> exec::timed_scheduler;
            { stdexec::get_completion_scheduler<stdexec::set_value_t>(stdexec::get_env(sock.Recv(rx))) } -> exec::timed_scheduler;
        };

    template<class T>
    concept CommonSocketConcept = SyncSocketConcept<T> || AsyncSocketConcept<T>;

    //! @brief Everything the Http1 read coroutines need besides the message itself,
    //! the transfer source, the intermediate-recv window, the ceilings and the exchange deadline.
    //! Handed down through the whole Read* family so no function ever opens its own
    //! recv loop (leftovers from one stage flow into the next one through
    //! @ref ExchangeBuffer).
    template<CommonSocketConcept SourceT>
    struct ReadContext {
        SourceT&        source;
        ExchangeBuffer& buffer;
        HttpLimits      limits{};

        //! @brief Absolute exchange deadline (`nullopt` = unbounded). Consumed by @ref Fill.
        std::optional<ExchangeDeadline> deadline{};
    };

#pragma endregion

}

#include <Thoth/Http/_base/Transport/_base.tpp>

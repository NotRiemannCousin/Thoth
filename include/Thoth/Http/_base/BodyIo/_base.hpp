#pragma once
#include <expected>
#include <span>

#include <Thoth/ThothError.hpp>


namespace Thoth::Http {

#pragma region Body Io

    //! @brief A read-only view over one body slice, the currency of the @ref BodyIo read side.
    using ByteView = std::span<const std::byte>;

    //! @brief Outcome of @ref BodyIo::Read, the next slice or `nullopt` once the body is drained.
    using ReadResult   = std::expected<std::optional<ByteView>, ThothError>;
    //! @brief Outcome of @ref BodyIo::Write, bytes consumed plus status (same shape as `Hermes::StreamByteOper`).
    using WriteResult  = std::pair<std::size_t, std::expected<void, ThothError>>;
    //! @brief Outcome of @ref BodyIo::FinishWrite, the commit/flush step of the incoming side.
    using FinishResult = std::expected<void, ThothError>;

    //! @brief The I/O protocol of a body, specialized per body type (same model as `std::formatter`).
    //!
    //! The body stays a dumb data holder (`std::string`, `std::vector<std::byte>`, a file handle, whatever), because
    //! nobody can add members to a type they do not own. All the "how do I push/pull bytes through this thing"
    //! knowledge lives on the specialization and so does the adaptation state (read cursor, bounce buffers), which
    //! means a fresh @ref BodyIo is all the reset a retried exchange needs.
    //!
    //! Two directions, one class. The direction concepts (@ref OutgoingBodyConcept, @ref IncomingBodyConcept) decide
    //! which members a body must provide and a body never needs to model both (symmetric ones like `std::string`
    //! simply specialize every member).
    //!
    //! | Member        | Direction | Contract                                                           |
    //! |---------------|-----------|--------------------------------------------------------------------|
    //! | `Size`        | outgoing  | payload size in bytes, `nullopt` when unknown without consuming    |
    //! | `Read`        | outgoing  | next slice or `nullopt`, at most one slice alive at a time         |
    //! | `Write`       | incoming  | consumes the window, returns the count (partial consumes are fine) |
    //! | `FinishWrite` | incoming  | commit/flush, called once, only after the last byte landed         |
    //!
    //! The wire framing (`Content-Length` vs `Transfer-Encoding: chunked`) is decided by the transport from `Size`,
    //! and the incoming side is blind to it (a decompressor does not care where the wire chunk ended).
    //!
    //! @note Contract sharp edges.
    //! - `Write` may consume less than the window (a sink with a fixed internal buffer does that), but ok with zero
    //!   consumed is forbidden, the transport treats it as a broken body and kills the exchange instead of spinning.
    //! - `Read` never yields an empty slice mid-body (a zero sized chunk on the wire means last-chunk), the transport
    //!   still skips empties defensively.
    //! - Every member may be async, in the sense of returning a sender instead of the plain result. The transport
    //!   normalizes both shapes through @ref Utils::Await at a single co_await.
    template<class Body>
    struct BodyIo;

    //! @brief Normalized alias to BodyIo.
    template<class Body>
    using BodyIoFor = BodyIo<std::remove_cvref_t<Body>>;

#pragma endregion


#pragma region Body Concepts

    //! @brief A body the transport pulls slices from on its way out (Client request, Server response).
    //! @see BodyIo.
    template<class Body>
    concept OutgoingBodyConcept =
        requires(BodyIoFor<Body>& io, const BodyIoFor<Body>& cio, const std::remove_cvref_t<Body>& body) {
        { cio.Size(body) } -> std::same_as<std::optional<std::uint64_t>>;
        io.Read(body);
        };

    //! @brief A body the transport pushes wire bytes into (Server request, Client response).
    //! @see BodyIo.
    template<class Body>
    concept IncomingBodyConcept =
        requires(BodyIoFor<Body>& io, std::remove_cvref_t<Body>& body, std::span<std::byte> window) {
        io.Write(body, window);
        io.FinishWrite(body);
        };



    template<class Body>
    concept BodyConcept = OutgoingBodyConcept<Body> || IncomingBodyConcept<Body>;

    template<class Body>
    concept SymetricBodyConcept = OutgoingBodyConcept<Body> && IncomingBodyConcept<Body>;



    template<class Body>
    concept DefaultIncomingBodyConcept = std::default_initializable<Body> && IncomingBodyConcept<Body>;

    template<class Body>
    concept DefaultOutgoingBodyConcept = std::default_initializable<Body> && OutgoingBodyConcept<Body>;

    template<class Body>
    concept DefaultBodyConcept         = std::default_initializable<Body> && BodyConcept<Body>;

    template<class Body>
    concept DefaultSymetricBodyConcept = std::default_initializable<Body> && SymetricBodyConcept<Body>;


    //! @brief A factory producing an incoming body before the first body byte is consumed, so the Server can refuse a
    //! body (factory error) without draining anything.
    template<class F, class Body, class Head>
    concept BodyFactoryConcept = IncomingBodyConcept<Body> &&
    requires (F f, const Head& head) {
        { std::invoke(f, head) } -> std::same_as<std::expected<Body, ThothError>>;
    };

#pragma endregion

}
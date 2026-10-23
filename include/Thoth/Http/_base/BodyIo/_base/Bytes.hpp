#pragma once
#include <cstdint>
#include <iterator>
#include <optional>
#include <ranges>
#include <span>
#include <variant>
#include <vector>

#include <Thoth/Http/_base/BodyIo/_base.hpp>
#include <Hermes/Socket/_base.hpp>

namespace Thoth::Http {
    //! @brief A range yielding byte-like elements directly.
    template<class T>
    concept DirectValueRange = std::ranges::input_range<T> && Hermes::ByteLike<std::ranges::range_value_t<T>>;

    //! @brief A range yielding byte-like slices.
    template<class T>
    concept ChunkValueRange = std::ranges::input_range<T> && DirectValueRange<std::ranges::range_value_t<T>>;

    template<class T>
    concept RangeBody = DirectValueRange<T> || ChunkValueRange<T>;


    //! @brief Customization point that teaches the BodyIo how to fill a range.
    //! You need to implement the "Write" and "FinishWrite" methods, just like the BodyIo, but both return void
    //! (a range sink always consumes the whole window).
    template<std::ranges::range T>
    struct BodyInfoBehavior;

    template<class R>
    concept HasBodyInfoBehaviorConcept = requires(R& r, std::span<std::byte> data) {
        { BodyInfoBehavior<R>::Write(r, data) } -> std::same_as<void>;
        { BodyInfoBehavior<R>::FinishWrite(r) } -> std::same_as<void>;
    };

    namespace details_ {
        //! @brief The const iterator of a range, or a placeholder when the range is not const iterable.
        template<class R>
        struct ConstIter { using type = std::monostate; };

        template<class R>
            requires std::ranges::range<const R>
        struct ConstIter<R> { using type = std::ranges::iterator_t<const R>; };
    }


    //! @brief Symmetric in-memory body over any range of bytes (`std::vector<std::byte>`, `std::deque<char>`, a
    //! vector of chunks...). The outgoing side needs a range that is const iterable, the incoming side needs a
    //! @ref BodyInfoBehavior for the range (growable value ranges have one out of the box).
    template<RangeBody R>
    struct BodyIo<R> {
        using RangeValueT = std::ranges::range_value_t<R>;

        //! @brief Window size of the bounce buffer, used when a slice cannot point straight at the range storage.
        static constexpr std::size_t k_bounceBytes{ 8 * 1024 };

        //! @brief Read cursor inside the current contiguous piece (the whole range for a value range, the current
        //! chunk for a chunk range).
        std::size_t pos{};

    private:
        using ChunkIter = details_::ConstIter<R>::type;

        //! @brief Current chunk of a chunk range, opened on the first Read (it points at the body).
        std::optional<ChunkIter> m_chunk{};
        std::vector<std::byte>   m_bounce{};

        //! @brief Next slice of one piece from @ref pos on. Zero copy when the piece is contiguous and stable
        //! (@p Stable), bounced into @ref k_bounceBytes windows otherwise. `nullopt` when the piece is drained.
        template<bool Stable, class Piece>
        std::optional<ByteView> ReadPiece(const Piece& piece);

    public:
        //! @brief Total payload size in bytes for sized value ranges, `nullopt` otherwise (chunk ranges go chunked).
        [[nodiscard]] std::optional<std::uint64_t> Size(const R& body) const;

        //! @brief The next slice of the range, empty chunks are skipped.
        ReadResult Read(const R& body)
            requires std::ranges::forward_range<const R>;

        //! @brief Hands the whole window to the @ref BodyInfoBehavior of the range.
        WriteResult Write(R& body, std::span<std::byte> window)
            requires HasBodyInfoBehaviorConcept<R>;

        //! @brief Lets the @ref BodyInfoBehavior of the range commit whatever it buffered.
        FinishResult FinishWrite(R& body)
            requires HasBodyInfoBehaviorConcept<R>;
    };
}

#include <Thoth/Http/_base/BodyIo/_base/Bytes.tpp>

static_assert(Thoth::Http::SymetricBodyConcept<std::vector<std::byte>>);
static_assert(Thoth::Http::OutgoingBodyConcept<std::vector<std::vector<std::byte>>>);
static_assert(!Thoth::Http::IncomingBodyConcept<std::vector<std::vector<std::byte>>>);
static_assert(!Thoth::Http::IncomingBodyConcept<std::span<const std::byte>>);

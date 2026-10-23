#pragma once
#include <algorithm>

namespace Thoth::Http {

    //! @brief Default behavior of growable value ranges, anything with a `push_back`.
    template<std::ranges::range R>
        requires DirectValueRange<R> && requires(R& r, std::ranges::range_value_t<R> v) { r.push_back(std::move(v)); }
    struct BodyInfoBehavior<R> {
        static void Write(R& body, std::span<std::byte> window) {
            using ValueT = std::ranges::range_value_t<R>;
            auto bytes{ window | std::views::transform([](std::byte b) { return static_cast<ValueT>(b); }) };

            if constexpr (requires { body.append_range(bytes); })
                body.append_range(bytes);
            else
                std::ranges::copy(bytes, std::back_inserter(body));
        }

        static void FinishWrite(R&) {}
    };


    template<RangeBody R>
    std::optional<std::uint64_t> BodyIo<R>::Size(const R& body) const {
        if constexpr (DirectValueRange<R> && std::ranges::sized_range<const R>)
            return static_cast<std::uint64_t>(std::ranges::size(body));
        else
            return std::nullopt;
    }

    template<RangeBody R>
    template<bool Stable, class Piece>
    std::optional<ByteView> BodyIo<R>::ReadPiece(const Piece& piece) {
        if constexpr (Stable && std::ranges::contiguous_range<const Piece>) {
            const auto size{ static_cast<std::size_t>(std::ranges::distance(piece)) };
            if (pos >= size)
                return std::nullopt;
            const auto* start{ reinterpret_cast<const std::byte*>(std::ranges::data(piece)) + pos };
            const auto left{ size - pos };
            pos = size;
            return ByteView{ start, left };
        }
        else {
            auto it{ std::ranges::next(std::ranges::begin(piece), static_cast<std::ptrdiff_t>(pos), std::ranges::end(piece)) };

            m_bounce.clear();
            m_bounce.reserve(k_bounceBytes);
            for (; it != std::ranges::end(piece) && m_bounce.size() < k_bounceBytes; ++it)
                m_bounce.push_back(static_cast<std::byte>(*it));

            pos += m_bounce.size();
            if (m_bounce.empty())
                return std::nullopt;
            return ByteView{ m_bounce };
        }
    }

    template<RangeBody R>
    ReadResult BodyIo<R>::Read(const R& body) requires std::ranges::forward_range<const R> {
        if constexpr (DirectValueRange<R>)
            return ReadPiece<true>(body);
        else {
            //! A chunk coming out of the iterator by value would die with it, so only references are sliced directly.
            constexpr bool stable{ std::is_lvalue_reference_v<std::ranges::range_reference_t<const R>> };

            if (!m_chunk) {
                m_chunk = std::ranges::begin(body);
                pos = 0;
            }

            while (*m_chunk != std::ranges::end(body)) {
                if (auto slice{ ReadPiece<stable>(**m_chunk) })
                    return slice;

                ++*m_chunk;
                pos = 0;
            }
            return std::optional<ByteView>{};
        }
    }

    template<RangeBody R>
    WriteResult BodyIo<R>::Write(R& body, std::span<std::byte> window) requires HasBodyInfoBehaviorConcept<R> {
        BodyInfoBehavior<R>::Write(body, window);
        return { window.size(), {} };
    }

    template<RangeBody R>
    FinishResult BodyIo<R>::FinishWrite(R& body) requires HasBodyInfoBehaviorConcept<R> {
        BodyInfoBehavior<R>::FinishWrite(body);
        return {};
    }
}

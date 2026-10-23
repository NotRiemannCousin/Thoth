#pragma once

namespace Thoth::Http {

    inline std::span<std::byte> ExchangeBuffer::Available() const noexcept {
        if (!buffer || readIdx >= filledIdx)
            return {};

        return { buffer->data() + readIdx, static_cast<std::size_t>(filledIdx - readIdx) };
    }

    inline std::span<std::byte> ExchangeBuffer::Free() noexcept {
        if (!buffer)
            buffer = std::make_unique<std::array<std::byte, k_capacity>>();

        return { buffer->data() + filledIdx, k_capacity - filledIdx };
    }

    inline void ExchangeBuffer::Commit(const std::size_t bytes) noexcept {
        filledIdx = static_cast<std::uint16_t>(filledIdx + bytes);
    }

    inline void ExchangeBuffer::Consume(std::size_t bytes) noexcept {
        readIdx = static_cast<std::uint16_t>(readIdx + bytes);
        if (readIdx == filledIdx)
            readIdx = filledIdx = 0;
    }
}
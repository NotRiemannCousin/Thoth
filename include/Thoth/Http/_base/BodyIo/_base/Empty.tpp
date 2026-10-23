#pragma once

namespace Thoth::Http {
    inline std::optional<std::uint64_t> BodyIo<EmptyBody>::Size(const EmptyBody&) const {
        return std::uint64_t{ 0 };
    }

    inline ReadResult BodyIo<EmptyBody>::Read(const EmptyBody&) {
        return std::optional<ByteView>{};
    }

    inline WriteResult BodyIo<EmptyBody>::Write(EmptyBody&, std::span<std::byte> window) {
        return { window.size(), {} };
    }

    inline FinishResult BodyIo<EmptyBody>::FinishWrite(EmptyBody&) {
        return {};
    }
}

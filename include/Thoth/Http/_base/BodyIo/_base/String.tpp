#pragma once

namespace Thoth::Http {
    inline std::optional<std::uint64_t> BodyIo<std::string>::Size(const std::string& body) const {
        return body.size();
    }

    inline ReadResult BodyIo<std::string>::Read(const std::string& body) {
        if (pos >= body.size())
            return std::optional<ByteView>{};
        const auto left{ body.size() - pos };
        const auto* start{ reinterpret_cast<const std::byte*>(body.data()) + pos };
        pos = body.size();
        return ByteView{ start, left };
    }

    inline WriteResult BodyIo<std::string>::Write(std::string& body, std::span<std::byte> window) {
        body.append(reinterpret_cast<const char*>(window.data()), window.size());
        return { window.size(), {} };
    }

    inline FinishResult BodyIo<std::string>::FinishWrite(std::string&) {
        return {};
    }
}

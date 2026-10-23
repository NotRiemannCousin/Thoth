#pragma once

#include <Thoth/Http/_base/BodyIo/_base.hpp>

namespace Thoth::Http {

    //! @brief A body with a known empty payload. `Size` reports zero (a valid `Content-Length: 0`, not the unknown of
    //! `nullopt`), Read ends immediately and the incoming side accepts and drops whatever arrives.
    struct EmptyBody {};

    template<>
    struct BodyIo<EmptyBody> {
        //! @brief Always zero.
        [[nodiscard]] std::optional<std::uint64_t> Size(const EmptyBody&) const;

        //! @brief Noop, returns immediately.
        ReadResult Read(const EmptyBody&);

        //! @brief Noop, returns immediately.
        WriteResult Write(EmptyBody&, std::span<std::byte> window);

        //! @brief Noop, returns immediately.
        FinishResult FinishWrite(EmptyBody&);
    };
}

#include <Thoth/Http/_base/BodyIo/_base/Empty.tpp>

static_assert(Thoth::Http::SymetricBodyConcept<Thoth::Http::EmptyBody>);
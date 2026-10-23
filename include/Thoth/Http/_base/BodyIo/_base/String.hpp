#pragma once
#include <string>

#include <Thoth/Http/_base/BodyIo/_base.hpp>

namespace Thoth::Http {

    //! @brief Symmetric in-memory text body. The whole payload is materialized, Read hands the remaining bytes out as
    //! a single zero-copy slice over the string storage and Write appends received bytes at the end.
    template<>
    struct BodyIo<std::string> {
        std::size_t pos{};

        //! @brief Total payload size in bytes.
        [[nodiscard]] std::optional<std::uint64_t> Size(const std::string& body) const;

        //! @brief The not-yet-sent tail of the string as one slice (zero copy).
        ReadResult Read(const std::string& body);

        //! @brief Appends the whole window. An in-memory sink always consumes everything at once.
        WriteResult Write(std::string& body, std::span<std::byte> window);

        //! @brief Nothing to flush on an in-memory sink.
        FinishResult FinishWrite(std::string&);
    };
}


#include <Thoth/Http/_base/BodyIo/_base/String.tpp>

static_assert(Thoth::Http::SymetricBodyConcept<std::string>);
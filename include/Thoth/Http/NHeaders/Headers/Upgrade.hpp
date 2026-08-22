#pragma once
#include <string>
#include <optional>

namespace Thoth::Http::NHeaders {
    //! @brief One protocol offered by the `Upgrade` header (RFC 9110 §7.8).
    //!
    //! The protocol name is stored in `protocol`, an optional slash-separated version is stored in `version`. For
    //! example, `websocket/13` becomes `{ "websocket", "13" }`.
    //!
    //! @par Example
    //! @code{.cpp}
    //! Upgrade websocket{ "websocket", "13" };
    //! @endcode
    struct Upgrade {
        //! Protocol name, such as `websocket`.
        std::string protocol{};
        //! Optional protocol version.
        std::optional<std::string> version{};

        bool operator==(const Upgrade& rhs) const noexcept = default;
    };
}

#include <Thoth/Http/NHeaders/Headers/Upgrade.tpp>

static_assert(Thoth::Utils::Serializable<Thoth::Http::NHeaders::Upgrade>);
#pragma once
#include <string_view>

namespace Thoth::Http {
    enum class VersionEnum : std::uint8_t { Http1_0, Http1_1, Http2, Http3, };

    //! @brief Exactly what you think it is.
    constexpr std::string_view VersionToString(VersionEnum version);
}

#include <Thoth/Http/_base/VersionEnum.tpp>

static_assert(std::formattable<Thoth::Http::VersionEnum, char>);
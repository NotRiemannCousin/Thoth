#pragma once

#include <Thoth/NJson/StringRef.hpp>

using namespace Thoth::NJson;

// NOLINTNEXTLINE(*)
StringRef::StringRef(const std::string_view str, const std::shared_ptr<std::string> _data) : str{ str }, _data{ _data } { }

StringRef::operator std::string_view() const noexcept {
    return str;
}


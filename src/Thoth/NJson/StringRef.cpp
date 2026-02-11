#pragma once

#include <Thoth/NJson/StringRef.hpp>

using namespace Thoth::NJson;

StringRef::StringRef(const std::string& other) : str{ other } { }

// NOLINTNEXTLINE(*)
StringRef::StringRef(const std::string_view other, const std::shared_ptr<std::string> _data) : str{ other }, _data{ _data } { }

StringRef::operator std::string_view() const noexcept {
    return str;
}

StringRef::operator std::string() const noexcept {
    return std::string{ str };
}

bool StringRef::operator==(const StringRef& other) const {
    return str == other.str;
}


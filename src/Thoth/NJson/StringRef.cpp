#include <Thoth/NJson/StringRef.hpp>

using namespace Thoth::NJson;

StringRef::StringRef(const std::string& str) :
        m_length{ static_cast<std::uint32_t>(str.size()) },
        m_data{ str.data(), [](const char*) noexcept {} } {}

StringRef::StringRef(const std::string_view other, const std::shared_ptr<const char[]>& base) noexcept :
        m_offset{ static_cast<std::uint32_t>(other.data() - base.get()) },
        m_length{ static_cast<std::uint32_t>(other.size()) },
        m_data{ base } {}

StringRef StringRef::FromOwned(std::string&& str) {
    const auto owner{ std::make_shared<std::string>(std::move(str)) };
    return StringRef{ std::string_view{ owner->data(), owner->size() },
                      std::shared_ptr<const char[]>{ owner, owner->data() } };
}

StringRef::operator std::string_view() const noexcept {
    return AsStringView();
}

StringRef::operator std::string() const {
    return std::string{ AsStringView() };
}

std::string_view StringRef::AsStringView() const noexcept {
    return std::string_view{ m_data.get() + m_offset, m_length };
}

bool StringRef::operator==(const StringRef& other) const noexcept {
    return AsStringView() == other;
}

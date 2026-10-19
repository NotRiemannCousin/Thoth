#include <Thoth/NJson/StringOwn.hpp>
#include <Thoth/NJson/StringRef.hpp>

namespace Thoth::NJson {

    StringOwn::StringOwn(std::string str)
        : m_data{ std::make_unique<std::string>(std::move(str)) } {}

    StringOwn::StringOwn(std::string_view str)
        : m_data{ std::make_unique<std::string>(str) } {}

    StringOwn::StringOwn(const char* str)
        : m_data{ str ? std::make_unique<std::string>(str) : nullptr } {}

    StringOwn::StringOwn(const StringRef& ref)
        : m_data{ std::make_unique<std::string>(static_cast<std::string_view>(ref)) } {}

    StringOwn::StringOwn(const StringOwn& other)
        : m_data{ other.m_data ? std::make_unique<std::string>(*other.m_data) : nullptr } {}

    StringOwn& StringOwn::operator=(const StringOwn& other) {
        if (this != &other) {
            if (other.m_data) {
                m_data = std::make_unique<std::string>(*other.m_data);
            } else {
                m_data.reset();
            }
        }
        return *this;
    }

    StringOwn& StringOwn::operator=(std::string str) {
        m_data = std::make_unique<std::string>(std::move(str));
        return *this;
    }

    StringOwn& StringOwn::operator=(std::string_view str) {
        m_data = std::make_unique<std::string>(str);
        return *this;
    }

    StringOwn::operator std::string() const noexcept {
        return m_data ? *m_data : std::string{};
    }

    bool StringOwn::operator==(const StringOwn& other) const noexcept {
        return view() == other.view();
    }

    bool StringOwn::operator==(std::string_view other) const noexcept {
        return view() == other;
    }

}
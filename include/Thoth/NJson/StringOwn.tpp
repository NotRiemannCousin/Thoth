#pragma once

namespace Thoth::NJson {

    inline StringOwn::operator std::string_view() const noexcept {
        return view();
    }

    inline std::string_view StringOwn::view() const noexcept {
        return m_data ? std::string_view{*m_data} : std::string_view{};
    }

    inline const std::string& StringOwn::string() const noexcept {
        static const std::string empty{};
        return m_data ? *m_data : empty;
    }

    inline std::string& StringOwn::string() noexcept {
        if (!m_data) {
            m_data = std::make_unique<std::string>();
        }
        return *m_data;
    }

}
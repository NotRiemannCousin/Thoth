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

#pragma region Range Interface

    inline auto StringOwn::begin()  const noexcept { return view().begin();  }
    inline auto StringOwn::end()    const noexcept { return view().end();    }
    inline auto StringOwn::cbegin() const noexcept { return view().cbegin(); }
    inline auto StringOwn::cend()   const noexcept { return view().cend();   }
    inline auto StringOwn::rbegin() const noexcept { return view().rbegin(); }
    inline auto StringOwn::rend()   const noexcept { return view().rend();   }

    inline auto StringOwn::size()   const noexcept { return view().size();   }
    inline auto StringOwn::empty()  const noexcept { return view().empty();  }
    inline auto StringOwn::data()   const noexcept { return view().data();   }

    inline auto StringOwn::front()  const noexcept { return view().front();  }
    inline auto StringOwn::back()   const noexcept { return view().back();   }

    template <typename Index>
    auto StringOwn::operator[](Index idx) const noexcept { return view()[idx]; }

    inline bool StringOwn::contains(std::string_view sv) const noexcept {
        return view().find(sv) != std::string_view::npos;
    }
    inline bool StringOwn::contains(char c) const noexcept {
        return view().find(c) != std::string_view::npos;
    }

#pragma endregion

}
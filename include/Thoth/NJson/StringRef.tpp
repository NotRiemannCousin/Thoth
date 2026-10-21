#pragma once
#include <format>
#include <Thoth/NJson/StringOwn.hpp>

namespace Thoth::NJson {
    inline StringRef::StringRef(const StringOwn& own) : StringRef{ own.string() } {}

#pragma region Range Interface

    constexpr auto StringRef::begin()  const noexcept { return AsStringView().begin();  }
    constexpr auto StringRef::end()    const noexcept { return AsStringView().end();    }
    constexpr auto StringRef::cbegin() const noexcept { return AsStringView().cbegin(); }
    constexpr auto StringRef::cend()   const noexcept { return AsStringView().cend();   }
    constexpr auto StringRef::rbegin() const noexcept { return AsStringView().rbegin(); }
    constexpr auto StringRef::rend()   const noexcept { return AsStringView().rend();   }

    constexpr auto StringRef::size()   const noexcept { return AsStringView().size();   }
    constexpr auto StringRef::empty()  const noexcept { return AsStringView().empty();  }
    constexpr auto StringRef::data()   const noexcept { return AsStringView().data();   }

    constexpr auto StringRef::front()  const noexcept { return AsStringView().front();  }
    constexpr auto StringRef::back()   const noexcept { return AsStringView().back();   }

    template <typename Index>
    constexpr auto StringRef::operator[](Index idx) const noexcept { return AsStringView()[idx]; }

    constexpr bool StringRef::contains(std::string_view sv) const noexcept { return AsStringView().find(sv) != std::string_view::npos; }
    constexpr bool StringRef::contains(char c) const noexcept { return AsStringView().find(c) != std::string_view::npos; }

#pragma endregion

}

template<>
struct std::formatter<Thoth::NJson::StringRef>{
    static constexpr auto parse(auto& ctx) {
        return std::formatter<std::string_view>{}.parse(ctx);
    }

    auto format(const Thoth::NJson::StringRef& ref, auto& ctx) const {
        return std::formatter<std::string_view>{}.format(static_cast<string_view>(ref), ctx);
    }
};


template<>
inline constexpr bool std::ranges::enable_borrowed_range<Thoth::NJson::StringRef> = true;
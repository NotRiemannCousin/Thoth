#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace Thoth::NJson {

    struct StringOwn;

    struct StringRef {
        using value_type      = char;
        using size_type       = std::string_view::size_type;
        using difference_type = std::string_view::difference_type;
        using pointer         = std::string_view::pointer;
        using const_pointer   = std::string_view::const_pointer;
        using reference       = value_type&;
        using const_reference = const value_type&;

        using iterator               = std::string_view::iterator;
        using const_iterator         = std::string_view::const_iterator;
        using reverse_iterator       = std::string_view::reverse_iterator;
        using const_reverse_iterator = std::string_view::const_reverse_iterator;

        explicit StringRef(const std::string& str);

        StringRef() noexcept = default;
        StringRef(StringRef&&) noexcept = default;
        StringRef(const StringRef&) noexcept = default;
        StringRef& operator=(StringRef&&) noexcept = default;
        StringRef& operator=(const StringRef&) noexcept = default;

        StringRef(std::string_view other, const std::shared_ptr<const char[]>& base) noexcept;

        explicit StringRef(const StringOwn& own);

        [[nodiscard]] static StringRef FromOwned(std::string&& str);

        operator std::string_view() const noexcept;
        operator std::string() const;

        std::string_view AsStringView() const noexcept;

        bool operator==(const StringRef& other) const noexcept;

#pragma region Range Interface

        [[nodiscard]] constexpr auto begin()  const noexcept;
        [[nodiscard]] constexpr auto end()    const noexcept;
        [[nodiscard]] constexpr auto cbegin() const noexcept;
        [[nodiscard]] constexpr auto cend()   const noexcept;
        [[nodiscard]] constexpr auto rbegin() const noexcept;
        [[nodiscard]] constexpr auto rend()   const noexcept;

        [[nodiscard]] constexpr auto size()   const noexcept;
        [[nodiscard]] constexpr auto empty()  const noexcept;
        [[nodiscard]] constexpr auto data()   const noexcept;

        [[nodiscard]] constexpr auto front()  const noexcept;
        [[nodiscard]] constexpr auto back()   const noexcept;

        template <typename Index>
        [[nodiscard]] constexpr auto operator[](Index idx) const noexcept;

        [[nodiscard]] constexpr bool contains(std::string_view sv) const noexcept;
        [[nodiscard]] constexpr bool contains(char c) const noexcept;

#pragma endregion

    private:
        std::uint32_t m_offset{};
        std::uint32_t m_length{};
        std::shared_ptr<const char[]> m_data;
    };

}

#include <Thoth/NJson/StringRef.tpp>

static_assert(std::ranges::range<Thoth::NJson::StringRef> && "StringOwn is not a range");
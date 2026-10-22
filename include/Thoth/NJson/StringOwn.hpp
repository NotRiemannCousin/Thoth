#pragma once
#include <memory>
#include <ranges>
#include <string>
#include <string_view>

namespace Thoth::NJson {
    struct StringRef;

    //! @brief Managed owned string wrapper designed to fit in 8 bytes (pointer size) with deep-copy semantics required
    //! by Copy-On-Write structures.
    struct StringOwn {
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

        StringOwn() noexcept = default;
        StringOwn(std::string str);
        StringOwn(std::string_view str);
        StringOwn(const char* str);

        StringOwn(const StringRef& ref);

        StringOwn(const StringOwn& other);
        StringOwn(StringOwn&& other) noexcept = default;

        StringOwn& operator=(const StringOwn& other);
        StringOwn& operator=(StringOwn&& other) noexcept = default;
        StringOwn& operator=(std::string str);
        StringOwn& operator=(std::string_view str);

        operator std::string_view() const noexcept;
        operator std::string() const noexcept;

        [[nodiscard]] std::string_view view() const noexcept;
        [[nodiscard]] const std::string& string() const noexcept;
        [[nodiscard]] std::string& string() noexcept;

        bool operator==(const StringOwn& other) const noexcept;
        bool operator==(std::string_view other) const noexcept;

#pragma region Range Interface

        [[nodiscard]] auto begin()  noexcept;
        [[nodiscard]] auto end()    noexcept;
        [[nodiscard]] auto rbegin() noexcept;
        [[nodiscard]] auto rend()   noexcept;

        [[nodiscard]] auto begin()  const noexcept;
        [[nodiscard]] auto end()    const noexcept;
        [[nodiscard]] auto rbegin() const noexcept;
        [[nodiscard]] auto rend()   const noexcept;

        [[nodiscard]] auto cbegin()  const noexcept;
        [[nodiscard]] auto cend()    const noexcept;
        [[nodiscard]] auto crbegin() const noexcept;
        [[nodiscard]] auto crend()   const noexcept;

        [[nodiscard]] auto size()  const noexcept;
        [[nodiscard]] auto empty() const noexcept;
        [[nodiscard]] auto data()  const noexcept;

        [[nodiscard]] auto front() const noexcept;
        [[nodiscard]] auto back()  const noexcept;

        template <typename Index>
        [[nodiscard]] auto operator[](Index idx) const noexcept;

        [[nodiscard]] bool contains(std::string_view sv) const noexcept;
        [[nodiscard]] bool contains(char c) const noexcept;

#pragma endregion

    private:
        std::unique_ptr<std::string> m_data;
    };
}

#include <Thoth/NJson/StringOwn.tpp>

static_assert(std::ranges::range<Thoth::NJson::StringOwn> && "StringOwn is not a range");
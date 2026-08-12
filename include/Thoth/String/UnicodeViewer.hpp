#pragma once
#include <Thoth/String/_base.hpp>
#include <string>

namespace Thoth::String {
    using Rune = char32_t;

    constexpr Rune UnknownChar{ U'\uFFFD' };


    //! @brief Non owning viewer that represents Unicode encodings.
    //!
    //! It goes only forward. Invalid chars are replaced by UnknownChar.
    template<UnicodeCharConcept CharT>
    struct UnicodeViewer {
        using  StringViewType = std::basic_string_view<CharT>;

        constexpr explicit UnicodeViewer(StringViewType str) : m_ref{ str } { }


        struct Iterator {
            using iterator_category = std::output_iterator_tag;
            using value_type = Rune;
            using difference_type = std::ptrdiff_t;

            constexpr Iterator() = default;
            constexpr explicit Iterator(StringViewType::const_iterator it, StringViewType::const_iterator end);

            [[nodiscard]] constexpr value_type operator*() const;
            [[nodiscard]] constexpr bool operator==(std::default_sentinel_t) const;
            constexpr Iterator& operator++();
            constexpr Iterator operator++(int);

        private:
            StringViewType::const_iterator m_curIt{};
            StringViewType::const_iterator m_end{};
            size_t     m_accInvalid{};
            value_type m_currValue{};
        };

        [[nodiscard]] constexpr Iterator begin() noexcept{        return Iterator{ m_ref.begin(), m_ref.end() }; }
        [[nodiscard]] constexpr Iterator cbegin() const noexcept{ return Iterator{ m_ref.begin(), m_ref.end() }; }
        [[nodiscard]] constexpr std::default_sentinel_t end() noexcept{        return std::default_sentinel_t{}; }
        [[nodiscard]] constexpr std::default_sentinel_t cend() const noexcept{ return std::default_sentinel_t{}; }

        //! @brief Check if a string does not has invalid chars.
        [[nodiscard]] static constexpr bool IsValid(StringViewType str);

        //! @brief Converts from one Unicode Encoding to another.
        template<UnicodeCharConcept NewCharT>
        static constexpr std::basic_string<NewCharT> ConvertTo(StringViewType str);
    private:
        StringViewType m_ref{};
    };

    using Utf8View  = UnicodeViewer<char8_t>;
    using Utf16View = UnicodeViewer<char16_t>;
    using Utf32View = UnicodeViewer<char32_t>;
}


#include <Thoth/String/UnicodeViewer.tpp>
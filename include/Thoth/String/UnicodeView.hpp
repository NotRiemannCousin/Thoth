#pragma once
#include <Thoth/String/_base.hpp>
#include <string>

namespace Thoth::String {
    using Rune = char32_t;

    constexpr Rune UnknownChar{ U'\uFFFD' };

    template<UnicodeCharConcept CharT>
    struct UnicodeView {
        using  StringViewType = std::basic_string_view<CharT>;

        constexpr explicit UnicodeView(StringViewType str) : _ref{ str } { }


        struct Iterator {
            using iterator_category = std::forward_iterator_tag;
            using value_type = Rune;
            using difference_type = std::ptrdiff_t;

            constexpr Iterator() = default;
            constexpr explicit Iterator(StringViewType::const_iterator it, StringViewType::const_iterator end);

            [[nodiscard]] constexpr value_type operator*() const;
            [[nodiscard]] constexpr bool operator==(Iterator) const;
            constexpr Iterator& operator++();
            constexpr Iterator operator++(int);

        private:
            StringViewType::const_iterator _it{};
            StringViewType::const_iterator _end{};
            size_t _accInvalid{};
            value_type _curr{};
        };

        [[nodiscard]] constexpr Iterator begin() noexcept{         return Iterator{ _ref.begin(), _ref.end() }; }
        [[nodiscard]] constexpr Iterator end() noexcept{           return Iterator{ _ref.end(), _ref.end() }; }
        [[nodiscard]] constexpr Iterator cbegin() const noexcept{  return Iterator{ _ref.begin(), _ref.end() }; }
        [[nodiscard]] constexpr Iterator cend() const noexcept{    return Iterator{ _ref.end(), _ref.end() }; }

        [[nodiscard]] static constexpr bool IsValid(StringViewType str);
        template<UnicodeCharConcept NewCharT>
        constexpr static std::basic_string<NewCharT> ConvertTo(StringViewType str);
    private:
        StringViewType _ref{};
    };

    using Utf8View  = UnicodeView<char8_t>;
    using Utf16View = UnicodeView<char16_t>;
    using Utf32View = UnicodeView<char32_t>;
}


#include <Thoth/String/UnicodeView.tpp>


namespace Thoth::String {
    static_assert(std::ranges::forward_range<Utf8View>);
    static_assert(std::ranges::forward_range<Utf16View>);
    static_assert(std::ranges::forward_range<Utf32View>);
}
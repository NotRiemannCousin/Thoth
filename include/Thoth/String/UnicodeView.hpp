#pragma once
#include <Thoth/String/_base.hpp>
#include <string>

namespace Thoth::String {
    using Rune = char32_t;

    constexpr Rune UnknownChar{ U'\uFFFD' };

    template<UnicodeCharConcept CharT>
    struct UnicodeView {
        using  StringViewType = std::basic_string_view<CharT>;

        explicit UnicodeView(StringViewType str) : _ref{ str } { }


        struct Iterator {
            using iterator_category = std::forward_iterator_tag;
            using value_type = Rune;
            using difference_type = std::ptrdiff_t;

            Iterator() = default;
            explicit Iterator(StringViewType::const_iterator it, StringViewType::const_iterator end);

            [[nodiscard]] value_type operator*() const;
            [[nodiscard]] bool operator==(Iterator) const;
            Iterator& operator++();
            Iterator operator++(int);

        private:
            StringViewType::const_iterator _it{};
            StringViewType::const_iterator _end{};
            size_t _accInvalid{};
            value_type _curr{};
        };

        [[nodiscard]] Iterator begin() noexcept{   return Iterator{ _ref.begin(), _ref.end() }; }
        [[nodiscard]] Iterator end() noexcept{     return Iterator{ _ref.end(), _ref.end() }; }
        [[nodiscard]] Iterator cbegin() const noexcept{  return Iterator{ _ref.begin(), _ref.end() }; }
        [[nodiscard]] Iterator cend() const noexcept{    return Iterator{ _ref.end(), _ref.end() }; }

        [[nodiscard]] static bool IsValid(StringViewType str);
        template<UnicodeCharConcept NewCharT>
        static std::basic_string<NewCharT> ConvertTo(StringViewType str);
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
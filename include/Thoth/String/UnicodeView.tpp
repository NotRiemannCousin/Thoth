#pragma once
#include <intrin.h>
#include <bit>

namespace Thoth::String {
    template<UnicodeCharConcept CharT>
    constexpr UnicodeView<CharT>::Iterator::Iterator(
            const typename StringViewType::const_iterator it,
            const typename StringViewType::const_iterator end
        ) : _it{ it }, _end{ end } {
        if (_it == _end)
            return;

        int count{ 1 };

        if constexpr (std::same_as<CharT, char32_t>) {
            _curr = *_it++;
        } else if constexpr (std::same_as<CharT, char16_t>) {
            auto firstUnit{ *_it++ };

            if (firstUnit >= 0xD800 && firstUnit <= 0xDBFF) {
                count++;

                if (_it == _end) //* missing low surrogate
                    goto error_with_count;

                auto secondUnit = *_it++;
                if (secondUnit < 0xDC00 || secondUnit > 0xDFFF) //* missing low surrogate
                    goto error_with_count;

                _curr = 0x10000 + ((firstUnit - 0xD800) << 10) + (secondUnit - 0xDC00);
            } else if (firstUnit >= 0xDC00 && firstUnit <= 0xDFFF)
                goto error_with_count;

            _curr = firstUnit;
        } else if constexpr (std::same_as<CharT, char8_t>) {
            const char8_t firstOct{ *_it++ };
            const int octCount{ std::countl_one(static_cast<unsigned char>(firstOct)) };

            if (octCount == 1 || octCount > 4)
                goto error_with_count;

            _curr = firstOct & (0xFF >> octCount); // cropping the 1's


            for (int i{ 1 }; i < octCount; i++) {
                if (_it == _end || *_it >> 6 != 0b10) //* check continuation sequence
                    goto error_with_count;

                _curr = _curr << 6 | *_it++ & 0b00111111;
                count++;
            }
        }

        if (_curr > 0x10FFFF        // above max valid
            || (_curr >= 0xD800 && _curr <= 0xDFFF)  // isolated surrogates
            || (_curr >= 0xFDD0 && _curr <= 0xFDEF)  // reserved noncharacters
            || (_curr & 0xFFFF) == 0xFFFE
            || (_curr & 0xFFFF) == 0xFFFF)
            error_with_count: _accInvalid = count;
    }

    template<UnicodeCharConcept CharT>
    constexpr bool UnicodeView<CharT>::Iterator::operator==(Iterator other) const {
        return _it == other._it && _accInvalid == other._accInvalid;
    }

    template<UnicodeCharConcept CharT>
    constexpr UnicodeView<CharT>::Iterator::value_type UnicodeView<CharT>::Iterator::operator*() const {
        if (_accInvalid)
            return UnknownChar;
        return _curr;
    }

    template<UnicodeCharConcept CharT>
    constexpr UnicodeView<CharT>::Iterator& UnicodeView<CharT>::Iterator::operator++() {
        if (_accInvalid) {
            --_accInvalid;
            return *this;
        }

        int count{ 1 };

        if constexpr (std::same_as<CharT, char32_t>) {
            _curr = *_it++;
        } else if constexpr (std::same_as<CharT, char16_t>) {
            auto firstUnit{ *_it++ };

            if (firstUnit >= 0xD800 && firstUnit <= 0xDBFF) {
                count++;

                if (_it == _end) //* missing low surrogate
                    goto error_with_count;

                auto secondUnit = *_it++;
                if (secondUnit < 0xDC00 || secondUnit > 0xDFFF) //* missing low surrogate
                    goto error_with_count;

                _curr = 0x10000 + ((firstUnit - 0xD800) << 10) + (secondUnit - 0xDC00);
            } else if (firstUnit >= 0xDC00 && firstUnit <= 0xDFFF)
                goto error_with_count;

            _curr = firstUnit;
        } else if constexpr (std::same_as<CharT, char8_t>) {
            const char8_t firstOct{ *_it++ };
            const int octCount{ std::countl_one(static_cast<unsigned char>(firstOct)) };

            if (octCount == 1 || octCount > 4)
                goto error_with_count;

            _curr = firstOct & (0xFF >> octCount); // cropping the 1's

            __assume(octCount < 4); // time is passing MSVC, where is `assume` attribute? putting `__` is weird

            for (int i{ 1 }; i < octCount; i++) {
                if (_it == _end || *_it >> 6 != 0b10) //* check continuation sequence
                    goto error_with_count;

                _curr = _curr << 6 | *_it++ & 0b00111111;
                count++;
            }
        }

    if (_curr > 0x10FFFF        // above max valid
        || (_curr >= 0xD800 && _curr <= 0xDFFF)  // isolated surrogates
        || (_curr >= 0xFDD0 && _curr <= 0xFDEF)  // reserved noncharacters
        || (_curr & 0xFFFF) == 0xFFFE
        || (_curr & 0xFFFF) == 0xFFFF)
            error_with_count: _accInvalid = count;

        return *this;
    }

    template<UnicodeCharConcept CharT>
    constexpr UnicodeView<CharT>::Iterator UnicodeView<CharT>::Iterator::operator++(int) {
        auto old{ *this };
        ++*this;

        return old;
    }

    template<UnicodeCharConcept CharT>
    constexpr bool UnicodeView<CharT>::IsValid(StringViewType str) {
        for (const auto& rune : UnicodeView{ str })
            if (rune == UnknownChar)
                return false;
        return true;
    }

    template<UnicodeCharConcept CharT>
    template<UnicodeCharConcept NewCharT>
    constexpr std::basic_string<NewCharT> UnicodeView<CharT>::ConvertTo(StringViewType str) {
        return {};
    }
}

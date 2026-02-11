#pragma once
#include <intrin.h>
#include <bit>

namespace Thoth::String {
    template<UnicodeCharConcept CharT>
    constexpr UnicodeViewer<CharT>::Iterator::Iterator(
            const typename StringViewType::const_iterator it,
            const typename StringViewType::const_iterator end
        ) : _curIt{ it }, _end{ end } {
        if (_curIt == _end)
            return;

        int count{ 1 };

        if constexpr (std::same_as<CharT, char32_t>) {
            _currValue = *_curIt;
        } else if constexpr (std::same_as<CharT, char16_t>) {
            auto firstUnit{ *_curIt };

            if ((firstUnit >= 0x0000 && firstUnit <= 0xD7FF)
                    || (firstUnit >= 0xE000 && firstUnit <= 0xFFFF)) //* the space is reserved to high surrogate
                _currValue = firstUnit;
            else if (firstUnit >= 0xD800 && firstUnit <= 0xDBFF) {
                ++count;

                if (++_curIt == _end) //* missing low surrogate
                    goto error_with_count;

                auto secondUnit{ *_curIt };
                if (secondUnit < 0xDC00 || secondUnit > 0xDFFF) //* missing low surrogate
                    goto error_with_count;

                _currValue = 0x10000 + ((firstUnit - 0xD800) << 10) + (secondUnit - 0xDC00);
            } else
                goto error_with_count;

        } else if constexpr (std::same_as<CharT, char8_t>) {
            const char8_t firstOct{ *_curIt };
            const int octCount{ std::countl_one(static_cast<unsigned char>(firstOct)) };

            if (octCount == 1 || octCount > 4)
                goto error_with_count;

            _currValue = firstOct & 0xFF >> octCount; // cropping the 1's


            for (int i{ 1 }; i < octCount; i++) {
                if (++_curIt == _end || *_curIt >> 6 != 0b10) //* checking continuation sequence
                    goto error_with_count;

                _currValue = _currValue << 6 | *_curIt & 0b00111111;
                count++;
            }
        }

        if (_currValue > 0x10FFFF        // above max valid
            || (_currValue >= 0xD800 && _currValue <= 0xDFFF)  // isolated surrogates
            || (_currValue >= 0xFDD0 && _currValue <= 0xFDEF)  // reserved noncharacters
            || (_currValue & 0xFFFF) == 0xFFFE
            || (_currValue & 0xFFFF) == 0xFFFF)
            error_with_count: _accInvalid = count;
    }

    template<UnicodeCharConcept CharT>
    constexpr bool UnicodeViewer<CharT>::Iterator::operator==(std::default_sentinel_t) const {
        return _curIt == _end && _accInvalid == 0;
    }

    template<UnicodeCharConcept CharT>
    constexpr UnicodeViewer<CharT>::Iterator::value_type UnicodeViewer<CharT>::Iterator::operator*() const {
        if (_accInvalid)
            return UnknownChar;
        return _currValue;
    }

    template<UnicodeCharConcept CharT>
    constexpr UnicodeViewer<CharT>::Iterator& UnicodeViewer<CharT>::Iterator::operator++() {
        if (_accInvalid) {
            --_accInvalid;
            return *this;
        }

        if (_curIt == _end) // ok, this is ugly, but I want to keep it bounded to _end
            return *this;
        if (++_curIt == _end)
            return *this;

        int count{ 1 };

        if constexpr (std::same_as<CharT, char32_t>) {
            _currValue = *_curIt;
        } else if constexpr (std::same_as<CharT, char16_t>) {
            auto firstUnit{ *_curIt };

            if ((firstUnit >= 0x0000 && firstUnit <= 0xD7FF)
                    || (firstUnit >= 0xE000 && firstUnit <= 0xFFFF)) //* the space is reserved to high surrogate
                        _currValue = firstUnit;
            else if (firstUnit >= 0xD800 && firstUnit <= 0xDBFF) {
                ++count;

                if (++_curIt == _end) //* missing low surrogate
                    goto error_with_count;

                auto secondUnit{ *_curIt };
                if (secondUnit < 0xDC00 || secondUnit > 0xDFFF) //* missing low surrogate
                    goto error_with_count;

                _currValue = 0x10000 + ((firstUnit - 0xD800) << 10) + (secondUnit - 0xDC00);
            } else
                goto error_with_count;

        } else if constexpr (std::same_as<CharT, char8_t>) {
            const char8_t firstOct{ *_curIt };
            const int octCount{ std::countl_one(static_cast<unsigned char>(firstOct)) };

            if (octCount == 1 || octCount > 4)
                goto error_with_count;

            _currValue = firstOct & 0xFF >> octCount; // cropping the 1's


            for (int i{ 1 }; i < octCount; i++) {
                if (++_curIt == _end || *_curIt >> 6 != 0b10) //* checking continuation sequence
                    goto error_with_count;

                _currValue = _currValue << 6 | *_curIt & 0b00111111;
                count++;
            }
        }

        if (_currValue > 0x10FFFF        // above max valid
            || (_currValue >= 0xD800 && _currValue <= 0xDFFF)  // isolated surrogates
            || (_currValue >= 0xFDD0 && _currValue <= 0xFDEF)  // reserved noncharacters
            || (_currValue & 0xFFFF) == 0xFFFE
            || (_currValue & 0xFFFF) == 0xFFFF)
            error_with_count: _accInvalid = count;

        return *this;
    }

    template<UnicodeCharConcept CharT>
    constexpr UnicodeViewer<CharT>::Iterator UnicodeViewer<CharT>::Iterator::operator++(int) {
        auto old{ *this };
        ++*this;

        return old;
    }

    template<UnicodeCharConcept CharT>
    constexpr bool UnicodeViewer<CharT>::IsValid(StringViewType str) {
        for (const auto& rune : UnicodeViewer{ str })
            if (rune == UnknownChar)
                return false;
        return true;
    }

    template<UnicodeCharConcept CharT>
    template<UnicodeCharConcept NewCharT>
    constexpr std::basic_string<NewCharT> UnicodeViewer<CharT>::ConvertTo(StringViewType str) {
        if constexpr (std::same_as<CharT, NewCharT>)
            return { str.data(), str.size() };
        if constexpr (std::same_as<NewCharT, char32_t>) {
            std::u32string res;
            for (auto c : UnicodeViewer{ str })
                res += c;
            return res;
        }

        std::basic_string<NewCharT> res;
        UnicodeViewer view{ str };

        for (auto rune : view) {
            if constexpr (std::same_as<NewCharT, char16_t>) {
                if (rune > 0xFFFF) {
                    res.push_back(static_cast<NewCharT>(0xD7C0u + (rune >> 10u)));
                    res.push_back(static_cast<NewCharT>(0xDC00u + (rune & 0x3FFu)));
                } else {
                    res.push_back(static_cast<NewCharT>(rune));
                }
            } else if constexpr (std::same_as<NewCharT, char8_t>) {
                constexpr char8_t seqMask{ 0b00111111 };
                constexpr char8_t seqMark{ 0b10000000 };

                const int type{ (rune > 0x7F) + (rune > 0x7FF) + (rune > 0xFFFF) };

                if (type == 0) {
                    res.push_back(static_cast<char8_t>(rune));
                    continue;
                }

                res.push_back(static_cast<char8_t>(((0b11110'000 << (3 - type)) | (rune >> (6 * type)))));


                for (int i{ type - 1 }; i >= 0; --i)
                    res.push_back(static_cast<char8_t>(seqMark | ((rune >> (6 * i)) & seqMask)));
            }
        }

        return res;
    }
}

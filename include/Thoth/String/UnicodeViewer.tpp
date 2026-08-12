#pragma once
#include <intrin.h>
#include <bit>

namespace Thoth::String {
    template<UnicodeCharConcept CharT>
    constexpr UnicodeViewer<CharT>::Iterator::Iterator(
            const typename StringViewType::const_iterator it,
            const typename StringViewType::const_iterator end
        ) : m_curIt{ it }, m_end{ end } {
        if (m_curIt == m_end)
            return;

        int count{ 1 };

        if constexpr (std::same_as<CharT, char32_t>) {
            m_currValue = *m_curIt;
        } else if constexpr (std::same_as<CharT, char16_t>) {
            auto firstUnit{ *m_curIt };

            if ((firstUnit >= 0x0000 && firstUnit <= 0xD7FF)
                    || (firstUnit >= 0xE000 && firstUnit <= 0xFFFF)) //* the space is reserved to high surrogate
                m_currValue = firstUnit;
            else if (firstUnit >= 0xD800 && firstUnit <= 0xDBFF) {
                ++count;

                if (++m_curIt == m_end) //* missing low surrogate
                    goto error_with_count;

                auto secondUnit{ *m_curIt };
                if (secondUnit < 0xDC00 || secondUnit > 0xDFFF) //* missing low surrogate
                    goto error_with_count;

                m_currValue = 0x10000 + ((firstUnit - 0xD800) << 10) + (secondUnit - 0xDC00);
            } else
                goto error_with_count;

        } else if constexpr (std::same_as<CharT, char8_t>) {
            const char8_t firstOct{ *m_curIt };
            if (firstOct >= 0xF5)
                goto error_with_count;

            const int octCount{ std::countl_one(static_cast<unsigned char>(firstOct)) };

            if (octCount == 1 || octCount > 4)
                goto error_with_count;

            m_currValue = firstOct & 0xFF >> octCount; // cropping the 1's

            for (int i{ 1 }; i < octCount; i++) {
                if (++m_curIt == m_end || *m_curIt >> 6 != 0b10) //* checking continuation sequence
                    goto error_with_count;

                m_currValue = m_currValue << 6 | *m_curIt & 0b00111111;
                count++;
            }
        }

        if (m_currValue > 0x10FFFF        // above max valid
            || (m_currValue >= 0xD800 && m_currValue <= 0xDFFF)  // isolated surrogates
            || (m_currValue >= 0xFDD0 && m_currValue <= 0xFDEF)  // reserved noncharacters
            || (m_currValue & 0xFFFF) == 0xFFFE
            || (m_currValue & 0xFFFF) == 0xFFFF)
            error_with_count: m_accInvalid = count;
    }

    template<UnicodeCharConcept CharT>
    constexpr bool UnicodeViewer<CharT>::Iterator::operator==(std::default_sentinel_t) const {
        return m_curIt == m_end && m_accInvalid == 0;
    }

    template<UnicodeCharConcept CharT>
    constexpr UnicodeViewer<CharT>::Iterator::value_type UnicodeViewer<CharT>::Iterator::operator*() const {
        if (m_accInvalid)
            return UnknownChar;
        return m_currValue;
    }

    template<UnicodeCharConcept CharT>
    constexpr UnicodeViewer<CharT>::Iterator& UnicodeViewer<CharT>::Iterator::operator++() {
        if (m_accInvalid) {
            --m_accInvalid;
            return *this;
        }

        if (m_curIt == m_end) // ok, this is ugly, but I want to keep it bounded to m_end
            return *this;
        if (++m_curIt == m_end)
            return *this;

        int count{ 1 };

        if constexpr (std::same_as<CharT, char32_t>) {
            m_currValue = *m_curIt;
        } else if constexpr (std::same_as<CharT, char16_t>) {
            auto firstUnit{ *m_curIt };

            if ((firstUnit >= 0x0000 && firstUnit <= 0xD7FF)
                    || (firstUnit >= 0xE000 && firstUnit <= 0xFFFF)) //* the space is reserved to high surrogate
                        m_currValue = firstUnit;
            else if (firstUnit >= 0xD800 && firstUnit <= 0xDBFF) {
                ++count;

                if (++m_curIt == m_end) //* missing low surrogate
                    goto error_with_count;

                auto secondUnit{ *m_curIt };
                if (secondUnit < 0xDC00 || secondUnit > 0xDFFF) //* missing low surrogate
                    goto error_with_count;

                m_currValue = 0x10000 + ((firstUnit - 0xD800) << 10) + (secondUnit - 0xDC00);
            } else
                goto error_with_count;

        } else if constexpr (std::same_as<CharT, char8_t>) {
            const char8_t firstOct{ *m_curIt };
            const int octCount{ std::countl_one(static_cast<unsigned char>(firstOct)) };

            if (octCount == 1 || octCount > 4)
                goto error_with_count;

            m_currValue = firstOct & 0xFF >> octCount; // cropping the 1's


            for (int i{ 1 }; i < octCount; i++) {
                if (++m_curIt == m_end || *m_curIt >> 6 != 0b10) //* checking continuation sequence
                    goto error_with_count;

                m_currValue = m_currValue << 6 | *m_curIt & 0b00111111;
                count++;
            }
        }

        if (m_currValue > 0x10FFFF        // above max valid
            || (m_currValue >= 0xD800 && m_currValue <= 0xDFFF)  // isolated surrogates
            || (m_currValue >= 0xFDD0 && m_currValue <= 0xFDEF)  // reserved noncharacters
            || (m_currValue & 0xFFFF) == 0xFFFE
            || (m_currValue & 0xFFFF) == 0xFFFF)
            error_with_count: m_accInvalid = count;

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
                constexpr char8_t k_seqMask{ 0b00111111 };
                constexpr char8_t k_seqMark{ 0b10000000 };

                const int type{ (rune > 0x7F) + (rune > 0x7FF) + (rune > 0xFFFF) };

                if (type == 0) {
                    res.push_back(static_cast<char8_t>(rune));
                    continue;
                }

                res.push_back(static_cast<char8_t>(((0b11110'000 << (3 - type)) | (rune >> (6 * type)))));


                for (int i{ type - 1 }; i >= 0; --i)
                    res.push_back(static_cast<char8_t>(k_seqMark | ((rune >> (6 * i)) & k_seqMask)));
            }
        }

        return res;
    }
}

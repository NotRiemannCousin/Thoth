#pragma once
#include <string_view>
#include <string>

namespace Thoth::String {
    struct CharSequences {
        struct Http {
            static constexpr std::string_view whitespace { " \t" };

            static constexpr std::string_view tchar {
                "abcdefghijklmnopqrstuvwxyz"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "0123456789"
                "!#$%&'*+-.^_`|~"
            };

            static constexpr std::string_view delimiters { "\"(),/:;<=>?@[\\]{}" };
        };

        //! @{ Whitespace & Control
        static constexpr std::string_view linearWhitespace { " \t" };
        static constexpr std::string_view whitespace       { " \t\n\v\f\r" };
        static constexpr std::string_view newlines         { "\r\n" };
        //! @}

        //! @{ Numbers & Letters
        static constexpr std::string_view digits       { "0123456789" };
        static constexpr std::string_view alphaLower   { "abcdefghijklmnopqrstuvwxyz" };
        static constexpr std::string_view alphaUpper   { "ABCDEFGHIJKLMNOPQRSTUVWXYZ" };
        static constexpr std::string_view alpha        { "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" };
        static constexpr std::string_view alphanumeric { "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789" };
        //! @}

        //! @{ Encodings
        static constexpr std::string_view hex           { "0123456789abcdefABCDEF" };
        static constexpr std::string_view hexLower      { "0123456789abcdef" };
        static constexpr std::string_view hexUpper      { "0123456789ABCDEF" };
        static constexpr std::string_view base64        { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };
        static constexpr std::string_view base64Url     { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_" };
        static constexpr std::string_view base64Padding { "=" };
        //! @}

        //! @{ Special & Filtering
        static constexpr std::string_view punctuation   { "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~" };
        static constexpr std::string_view nonPrintable  {
            "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f"
            "\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x7f"
        };
        //! @}
    };

    bool CaseInsensitiveCompare(char c1, char c2);

    void Trim(std::string_view& str, std::string_view trim = CharSequences::whitespace);
    void LeftTrim(std::string_view& str, std::string_view trim = CharSequences::whitespace);
    void RightTrim(std::string_view& str, std::string_view trim = CharSequences::whitespace);

    std::string_view Trimmed(std::string_view str, std::string_view trim = CharSequences::whitespace);
    std::string_view LeftTrimmed(std::string_view str, std::string_view trim = CharSequences::whitespace);
    std::string_view RightTrimmed(std::string_view str, std::string_view trim = CharSequences::whitespace);

    std::string TrimmedStr(std::string_view str, std::string_view trim = CharSequences::whitespace);
    std::string LeftTrimmedStr(std::string_view str, std::string_view trim = CharSequences::whitespace);
    std::string RightTrimmedStr(std::string_view str, std::string_view trim = CharSequences::whitespace);



    bool IsVisible(char c);
}

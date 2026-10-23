#include <Thoth/String/Utils.hpp>


char Thoth::String::ToLower(const char c) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
}

char Thoth::String::ToUpper(const char c) {
    return static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
}

bool Thoth::String::CaseInsensitiveCompare(const char c1, const char c2) {
    return ToLower(c1) == ToLower(c2);
}

std::string_view Thoth::String::AsView(const std::string_view str) {
    return str;
}

void Thoth::String::Trim(std::string_view &str, const std::string_view trim) {
    LeftTrim(str, trim);
    RightTrim(str, trim);
}

void Thoth::String::LeftTrim(std::string_view &str, const std::string_view trim) {
    while (!str.empty() && trim.contains(str.front()))
        str.remove_prefix(1);
}

void Thoth::String::RightTrim(std::string_view &str, const std::string_view trim) {
    while (!str.empty() && trim.contains(str.back()))
        str.remove_suffix(1);
}



std::string_view Thoth::String::Trimmed(const std::string_view str, const std::string_view trim) {
    std::string_view s{ str };
    Trim(s, trim);
    // ReSharper disable once CppDFALocalValueEscapesFunction
    return s;
}

std::string_view Thoth::String::LeftTrimmed(const std::string_view str, const std::string_view trim) {
    std::string_view s{ str };
    LeftTrim(s, trim);
    return s;
}

std::string_view Thoth::String::RightTrimmed(const std::string_view str, const std::string_view trim) {
    std::string_view s{ str };
    RightTrim(s, trim);
    return s;
}


std::string Thoth::String::TrimmedStr(const std::string_view str, const std::string_view trim) {
    return std::string{ Trimmed(str, trim) };
}

std::string Thoth::String::LeftTrimmedStr(const std::string_view str, const std::string_view trim) {
    return std::string{ LeftTrimmed(str, trim) };
}

std::string Thoth::String::RightTrimmedStr(const std::string_view str, const std::string_view trim) {
    return std::string{ RightTrimmed(str, trim) };
}

bool Thoth::String::IsVisible(char c) {
    return c >= 0x21 && c <= 0x7E;
}

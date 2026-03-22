#include <Thoth/String/Utils.hpp>


void Thoth::String::Trim(std::string_view &str, std::string_view trim) {
    LeftTrim(str, trim);
    RightTrim(str, trim);
}

void Thoth::String::LeftTrim(std::string_view &str, std::string_view trim) {
    while (!str.empty() && trim.contains(str.front()))
        str.remove_prefix(1);
}

void Thoth::String::RightTrim(std::string_view &str, std::string_view trim) {
    while (!str.empty() && trim.contains(str.back()))
        str.remove_suffix(1);
}



std::string_view Thoth::String::Trimmed(std::string_view str, std::string_view trim) {
    std::string_view s{ str };
    Trim(s);
    // ReSharper disable once CppDFALocalValueEscapesFunction
    return s;
}

std::string_view Thoth::String::LeftTrimmed(std::string_view str, std::string_view trim) {
    std::string_view s{ str };
    LeftTrim(s);
    // ReSharper disable once CppDFALocalValueEscapesFunction
    return s;
}

std::string_view Thoth::String::RightTrimmed(std::string_view str, std::string_view trim) {
    std::string_view s{ str };
    RightTrim(s);
    // ReSharper disable once CppDFALocalValueEscapesFunction
    return s;
}


std::string Thoth::String::TrimmedStr(std::string_view str, std::string_view trim) {
    return std::string{ Trimmed(str) };
}

std::string Thoth::String::LeftTrimmedStr(std::string_view str, std::string_view trim) {
    return std::string{ LeftTrimmed(str) };
}

std::string Thoth::String::RightTrimmedStr(std::string_view str, std::string_view trim) {
    return std::string{ RightTrimmed(str) };
}

bool Thoth::String::IsVisible(char c) {
    return c >= 0x21 && c <= 0x7E;
}

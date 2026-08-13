#include <algorithm>
#include <Thoth/Http/NHeaders/Headers/MimeType.hpp>

using Thoth::Http::NHeaders::MimeType;

bool MimeType::operator==(const MimeType &other) const {
    // Parameter names are case-insensitive (RFC 9110 §8.3.1) and their order carries
    // no meaning, so "a=1;b=2" must compare equal to "B=2;A=1". Values are kept
    // case-sensitive since not every parameter's value is case-insensitive.
    static constexpr auto optionEqual{ [](const auto &a, const auto &b) {
        return std::ranges::equal(a.first, b.first, String::CaseInsensitiveCompare)
            && a.second == b.second;
    } };

    return std::ranges::equal(String::Trimmed(type)   , String::Trimmed(other.type)   , String::CaseInsensitiveCompare)
        && std::ranges::equal(String::Trimmed(subtype), String::Trimmed(other.subtype), String::CaseInsensitiveCompare)
        && std::ranges::is_permutation(options, other.options, optionEqual);
}
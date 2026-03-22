#include <algorithm>
#include <Thoth/Http/NHeaders/Headers/MimeType.hpp>

using Thoth::Http::NHeaders::MimeType;

bool MimeType::operator==(const MimeType &other) const {
    return options == other.options
        && std::ranges::equal(String::Trimmed(type)   , String::Trimmed(other.type)   , String::CaseInsensitiveCompare)
        && std::ranges::equal(String::Trimmed(subtype), String::Trimmed(other.subtype), String::CaseInsensitiveCompare);
}
#pragma once
#include <concepts>

namespace Thoth::String {
    template<class C>
    concept UnicodeCharConcept =
        std::same_as<C, char8_t> ||
        std::same_as<C, char16_t> ||
        std::same_as<C, char32_t>;
}

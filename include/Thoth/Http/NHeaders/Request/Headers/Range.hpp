#pragma once
#include <optional>
#include <variant>

namespace Thoth::Http::NHeaders {
    struct PrefixedRange {
        unsigned int start;
        std::optional<unsigned int> end; // make start + count to eliminate invalid state?
    };
    struct SuffixedRange {
        unsigned int last;
    };
    using Range = std::variant<PrefixedRange, SuffixedRange>;
}


#include <Thoth/Http/NHeaders/Request/Headers/Range.tpp>

static_assert(Thoth::Utils::Serializable<Thoth::Http::NHeaders::Range>);
#pragma once
#include <optional>
#include <Thoth/Http/NHeaders/_base.hpp>

namespace Thoth::Http::NHeaders {
    struct MimeType {
        string type;
        string subtype;
        std::vector<std::pair<string, string>> options;
    };
}

#include <Thoth/Http/NHeaders/Headers/MimeType.tpp>

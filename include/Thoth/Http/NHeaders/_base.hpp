#pragma once
#include <string>
#include <vector>

namespace Thoth::Http::NHeaders {
    using HeaderKey      = std::string;
    using HeaderKeyRef   = std::string_view;

    using HeaderValue    = std::string;
    using HeaderValueRef = std::string_view;

    // Well, it really maps to something, but isn't a map. The name will be maintained
    // to not break the naming convention of this lib.
    using HeaderPair     = std::pair<HeaderKey, HeaderValue>;
    using HeaderPairRef  = std::pair<HeaderKeyRef, HeaderValueRef>;
    using MapType        = std::vector<HeaderPair>;

}
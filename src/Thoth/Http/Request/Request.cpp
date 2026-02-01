#include <Thoth/Http/Request/Request.hpp>

std::string_view Thoth::Http::VersionToString(VersionEnum version) {
    switch (version) {
        case VersionEnum::HTTP1_0: return "HTTP/1.0";
        case VersionEnum::HTTP1_1: return "HTTP/1.1";
        case VersionEnum::HTTP2:   return "HTTP/2";
        case VersionEnum::HTTP3:   return "HTTP/3";
        default: return "HTTP/1.1";
    }
};

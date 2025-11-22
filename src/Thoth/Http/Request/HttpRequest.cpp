#include <Thoth/Http/Request/Request.hpp>

std::string_view Thoth::Http::VersionToString(Version version) {
    switch (version) {
        case Version::HTTP1_0: return "HTTP/1.0";
        case Version::HTTP1_1: return "HTTP/1.1";
        case Version::HTTP2:   return "HTTP/2";
        case Version::HTTP3:   return "HTTP/3";
        default: return "HTTP/1.1";
    }
};

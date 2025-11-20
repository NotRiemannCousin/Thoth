#include <Thoth/Http/Request/HttpRequest.hpp>

std::string_view Thoth::Http::HttpVersionToString(HttpVersion version) {
    switch (version) {
        case HttpVersion::HTTP1_0: return "HTTP/1.0";
        case HttpVersion::HTTP1_1: return "HTTP/1.1";
        case HttpVersion::HTTP2:   return "HTTP/2";
        case HttpVersion::HTTP3:   return "HTTP/3";
        default: return "HTTP/1.1";
    }
};

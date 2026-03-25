#include <Thoth/Http/NHeaders/Response/ResponseHeaders.hpp>

using namespace Thoth::Http::NHeaders;

ValueProxy<false, AcceptRanges> Thoth::Http::ResponseHeaders::AcceptRanges() {
    return { "accept-ranges", *this };
}

ValueProxy<true, AcceptRanges> Thoth::Http::ResponseHeaders::AcceptRanges() const {
    return { "accept-ranges", *this };
}

ValueProxy<false, std::chrono::seconds> Thoth::Http::ResponseHeaders::Age() {
    return { "age", *this }; // %Q for outPattern
}

ValueProxy<true, std::chrono::seconds> Thoth::Http::ResponseHeaders::Age() const {
    return { "age", *this }; // %Q for outPattern
}

ValueProxy<false, EntityTag> Thoth::Http::ResponseHeaders::EntityTag() {
    return { "etag", *this };
}

ValueProxy<true, EntityTag> Thoth::Http::ResponseHeaders::EntityTag() const {
    return { "etag", *this };
}

ValueProxy<false, std::string> Thoth::Http::ResponseHeaders::Location() {
    return { "location", *this };
}

ValueProxy<true, std::string> Thoth::Http::ResponseHeaders::Location() const {
    return { "location", *this };
}

ValueProxy<false, std::string> Thoth::Http::ResponseHeaders::ProxyAuthenticate() {
    return { "proxy-authenticate", *this };
}

ValueProxy<true, std::string> Thoth::Http::ResponseHeaders::ProxyAuthenticate() const {
    return { "proxy-authenticate", *this };
}

ValueProxy<false, std::chrono::utc_clock::time_point, std::chrono::seconds> Thoth::Http::ResponseHeaders::RetryAfter() {
    return { "retry-after", *this }; // %Q for outPattern
}

ValueProxy<true, std::chrono::utc_clock::time_point, std::chrono::seconds> Thoth::Http::ResponseHeaders::RetryAfter() const {
    return { "retry-after", *this }; // %Q for outPattern
}

ValueProxy<false, std::string> Thoth::Http::ResponseHeaders::Server() {
    return { "server", *this };
}

ValueProxy<true, std::string> Thoth::Http::ResponseHeaders::Server() const {
    return { "server", *this };
}

ListProxy<false, std::string> Thoth::Http::ResponseHeaders::Vary() {
    return { "vary", *this };
}

ListProxy<true, std::string> Thoth::Http::ResponseHeaders::Vary() const {
    return { "vary", *this };
}

ValueProxy<false, std::string> Thoth::Http::ResponseHeaders::WwwAuthenticate() {
    return { "www-authenticate", *this };
}

ValueProxy<true, std::string> Thoth::Http::ResponseHeaders::WwwAuthenticate() const {
    return { "www-authenticate", *this };
}
#include <Thoth/Http/NHeaders/Request/RequestHeaders.hpp>

using namespace Thoth::Http::NHeaders;
using namespace Thoth::Http;

// //! not parse nor validate data.
// auto RequestHeaders::GetNonValidatedView() const;
// //! @warning Risk of dangling reference if the underlying collection is modified.
// auto RequestHeaders::GetCookiesView() const;

ValueProxy<false, std::string> RequestHeaders::Authorization() {
    return { "authorization", *this };
}
ValueProxy<true, std::string> RequestHeaders::Authorization() const {
    return { "authorization", *this };
}
ValueProxy<false, RequestHeaders::Url> RequestHeaders::Host() {
    return { "host", *this };
}

ValueProxy<true, RequestHeaders::Url> RequestHeaders::Host() const {
    return { "host", *this };
}
ValueProxy<false, RequestHeaders::Url> RequestHeaders::Referrer() {
    return { "referrer", *this };
}
ValueProxy<true, RequestHeaders::Url> RequestHeaders::Referrer() const {
    return { "referrer", *this };
}
ValueProxy<false, RequestHeaders::Url, std::monostate> RequestHeaders::Origin() {
    return { "origin", *this };
}
ValueProxy<true, RequestHeaders::Url, std::monostate> RequestHeaders::Origin() const {
    return { "origin", *this };
};
ValueProxy<false, std::string> RequestHeaders::From() {
    return { "from", *this };
}
ValueProxy<true, std::string> RequestHeaders::From() const {
    return { "from", *this };
}
ValueProxy<false, unsigned int> RequestHeaders::MaxForwards() {
    return { "max-forwards", *this };
}
ValueProxy<true, unsigned int> RequestHeaders::MaxForwards() const {
    return { "max-forwards", *this };
}
//void RequestHeaders::GetProtocol();
//void RequestHeaders::GetProtocol() const;
ValueProxy<false, std::string> RequestHeaders::ProxyAuthorization() {
    return { "proxy-authorization", *this };
}
ValueProxy<true, std::string> RequestHeaders::ProxyAuthorization() const {
    return { "proxy-authorization", *this };
}
ListProxy<false, Range> RequestHeaders::Range() {
    return { "range", *this };
}
ListProxy<true, Range> RequestHeaders::Range() const {
    return { "range", *this };
}

// ValueProxy<false, std::string> RequestHeaders::CacheControl();
// ValueProxy<true, std::string> RequestHeaders::CacheControl() const;

ValueProxy<false, std::chrono::utc_clock::time_point> RequestHeaders::IfModifiedSince() {
    return { "if-modified-since", *this };
}
ValueProxy<true, std::chrono::utc_clock::time_point> RequestHeaders::IfModifiedSince() const {
    return { "if-modified-since", *this };
}
ValueProxy<false, std::chrono::utc_clock::time_point> RequestHeaders::IfUnmodifiedSince() {
    return { "if-unmodified-since", *this };
}
ValueProxy<true, std::chrono::utc_clock::time_point> RequestHeaders::IfUnmodifiedSince() const {
    return { "if-unmodified-since", *this };
}
ValueProxy<false, std::chrono::utc_clock::time_point, std::string> RequestHeaders::IfRange() {
    return { "if-range", *this };
}
ValueProxy<true, std::chrono::utc_clock::time_point, std::string> RequestHeaders::IfRange() const {
    return { "if-range", *this };
}
ListProxy<false, EntityTag> RequestHeaders::IfMatch() {
    return { "if-match", *this };
}
ListProxy<true, EntityTag> RequestHeaders::IfMatch() const {
    return { "if-match", *this };
}
ValueProxy<false, EntityTag> RequestHeaders::IfNoneMatch() {
    return { "if-none-match", *this };
}
ValueProxy<true, EntityTag> RequestHeaders::IfNoneMatch() const {
    return { "if-none-match", *this };
}
ListProxy<false, std::string> RequestHeaders::AcceptLanguage() {
    return { "accept-language", *this };
}
ListProxy<true, std::string> RequestHeaders::AcceptLanguage() const {
    return { "accept-language", *this };
}
ListProxy<false, TeEnum> RequestHeaders::Te() {
    return { "te", *this };
}
ListProxy<true, TeEnum> RequestHeaders::Te() const {
    return { "te", *this };
}


//         ListProxy<true, std::string>V<alueProxy<, std::string> RequestHeaders::AccessControlAllowCredentials();
//         ListProxy<false, std::string>ValueProxytruefalse, std::string> RequestHeaders::AccessControlAllowCredentials() const;
//         ValueProxy<false, std::string> RequestHeaders::AccessControlAllowHeaders();
//         ValueProxy<true, std::string> RequestHeaders::AccessControlAllowHeaders() const;
//         ValueProxy<false, std::string> RequestHeaders::AccessControlAllowMethods();
//         ValueProxy<true, std::string> RequestHeaders::AccessControlAllowMethods() const;
//         ValueProxy<false, std::string> RequestHeaders::AccessControlAllowOrigin();
//         ValueProxy<true, std::string> RequestHeaders::AccessControlAllowOrigin() const;
//         ValueProxy<false, std::string> RequestHeaders::AccessControlExposeAge();
//         ValueProxy<true, std::string> RequestHeaders::AccessControlExposeAge() const;
//         ValueProxy<false, std::string> RequestHeaders::AccessControlExposeMethods();
//         ValueProxy<true, std::string> RequestHeaders::AccessControlExposeMethods() const;
//         ValueProxy<false, std::string> RequestHeaders::AccessControlRequestMethod();
//         ValueProxy<true, std::string> RequestHeaders::AccessControlRequestMethod() const;
//         ValueProxy<false, std::string> RequestHeaders::AccessControlRequestHeaders();
//         ValueProxy<true, std::string> RequestHeaders::AccessControlRequestHeaders() const;
#pragma once


namespace Thoth::Http {
    // template<HttpMethodConcept Method>
    // expected<HttpRequest<Method>, string> HttpRequest<Method>::Connect(string_view url) {
    //     // return HttpUrl::FromUrl(url)
    //     //         .or_else([]{ return std::unexpected{"Invalid URL"}; })
    //     //         .and_then(Connect);
    //     return {};
    // }
    //
    // template<HttpMethodConcept Method>
    // expected<HttpRequest<Method>, string> HttpRequest<Method>::Connect(const HttpUrl& url) {
    //     return {};
    // }
}
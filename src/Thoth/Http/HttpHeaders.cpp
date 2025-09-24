#include <Thoth/Http/HttpHeaders.hpp>

#include <algorithm>
#include <ranges>


namespace Thoth::Http {
    using std::string_view;
    using std::string;
    namespace rg = std::ranges;
    namespace vs = std::views;

    struct HttpUrl;

    constexpr char ToLower(char c) {
        if ('A' <= c && c <= 'Z')
            return c - 'A' + 'a';
        return c;
    }

    constexpr auto inline HeaderSanitize    = vs::transform(ToLower);
    constexpr auto inline HeaderSanitizeStr = HeaderSanitize | rg::to<string>();
    // TODO: reduce string materialization                   like this ^^^
    // ? Create a new comparable_view?

    HttpHeaders::HttpHeaders(const MapType& initAs) {
        _headers.reserve(initAs.size());

        for (const auto& [key, val] : initAs)
            _headers.emplace_back(key | HeaderSanitizeStr, val);
    }

    HttpHeaders::HttpHeaders(const std::initializer_list<HeaderPair> &init) {
        _headers.reserve(init.size());

        for (const auto& [key, val] : init)
            _headers.emplace_back(key | HeaderSanitizeStr, val);
    }




    bool HttpHeaders::Exists(HeaderKeyRef key) const {
        return rg::contains(_headers | vs::keys, key | HeaderSanitizeStr);
    }

    bool HttpHeaders::Exists(HeaderPairRef p) const {
        return rg::contains(_headers, std::pair{ p.first | HeaderSanitizeStr, p.second });
    }

    bool HttpHeaders::Exists(HeaderKeyRef key, HeaderValueRef val) const {
        return Exists({key, val});
    }


    void HttpHeaders::Add(HeaderPairRef p) {
        auto key{ p.first | HeaderSanitizeStr };
        if (key != "set-cookie")
            if (auto it{ rg::find(_headers, key, &HeaderPair::first) }; it != _headers.end()) {
#ifdef __cpp_lib_ranges_concat

                it->second = vs::concat(it->second, vs::single(", "), p.second);
#else
                it->second += ", " + p.second;
#endif
                return;
            }


        _headers.emplace_back(std::move(key), p.second);
    }

    void HttpHeaders::Add(HeaderKeyRef key, HeaderValueRef val) {
        Add({key, val});
    }

    void HttpHeaders::Set(HeaderPairRef p) {
        auto key{ p.first | HeaderSanitizeStr };
        if (key != "set-cookie")
            if (auto it{ rg::find(_headers, key, &HeaderPair::first) }; it != _headers.end()) {
                it->second = p.second;
                return;
            }


        _headers.emplace_back(std::move(key), p.second);
    }

    void HttpHeaders::Set(HeaderKeyRef key, HeaderValueRef val) {
        Set({key, val});
    }

    bool HttpHeaders::Remove(HeaderPairRef p) {
        auto&& it{ rg::find(_headers, std::pair{ p.first | HeaderSanitizeStr, p.second }) };
        if (it == _headers.end())
            return false;

        _headers.erase(it);

        return true;
    }

    bool HttpHeaders::Remove(HeaderKeyRef key, HeaderValueRef val) {
        return Remove({key, val});
    }

    bool HttpHeaders::SetIfNull(HeaderPairRef p) {
        if (Exists(p.first))
            return false;

        Set(p);
        return true;
    }

    bool HttpHeaders::SetIfNull(HeaderKeyRef key, HeaderValueRef val) {
        return SetIfNull({key, val});
    }


    std::optional<std::reference_wrapper<HttpHeaders::HeaderValue>> HttpHeaders::Get(HeaderKeyRef key) {
        const auto it{  rg::find(_headers, key | HeaderSanitizeStr, &HeaderPair::first) };

        if (it != _headers.end())
            return it->second;

        return std::nullopt;
    }

    std::optional<std::reference_wrapper<const HttpHeaders::HeaderValue>> HttpHeaders::Get(HeaderKeyRef key) const {
        const auto it{  rg::find(_headers, key | HeaderSanitizeStr, &HeaderPair::first) };

        if (it != _headers.end())
            return it->second;

        return std::nullopt;
    }


    std::vector<HttpHeaders::HeaderValue> HttpHeaders::GetSetCookie() const {
        return GetSetCookieView() | rg::to<std::vector>();
    }


    HttpHeaders::IterType HttpHeaders::begin() { return _headers.begin(); }

    HttpHeaders::IterType HttpHeaders::end() { return _headers.end(); }

    HttpHeaders::CIterType HttpHeaders::begin() const { return _headers.cbegin(); }

    HttpHeaders::CIterType HttpHeaders::end() const { return _headers.cend(); }

    HttpHeaders::RIterType HttpHeaders::rbegin() { return _headers.rbegin(); }

    HttpHeaders::RIterType HttpHeaders::rend() { return _headers.rend(); }

    HttpHeaders::CRIterType HttpHeaders::rbegin() const { return _headers.crbegin(); }

    HttpHeaders::CRIterType HttpHeaders::rend() const { return _headers.crend(); }



    void HttpHeaders::Clear() { _headers.clear(); }

    size_t HttpHeaders::Size() const { return _headers.size(); }

    bool HttpHeaders::Empty() const { return _headers.size() == 0; }



    HttpHeaders::HeaderValue& HttpHeaders::operator[](HeaderKeyRef key) {
        const auto newKey{ key | HeaderSanitizeStr };
        if (const auto it{  rg::find(_headers, newKey, &HeaderPair::first) }; it != _headers.end())
            return it->second;

        _headers.emplace_back(std::move(newKey), string{});
        return _headers.back().second;
    }

    bool HttpHeaders::operator==(const HttpHeaders& other) const = default;
}

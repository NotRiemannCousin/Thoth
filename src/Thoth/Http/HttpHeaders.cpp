#include <Thoth/Http/HttpHeaders.hpp>

#include <algorithm>
#include <bitset>
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

    HttpHeaders::HttpHeaders() = default;

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

    WebResult<HttpHeaders> HttpHeaders::Parse(std::string_view headers) {
        constexpr auto isCharAllowed = [](char c) {
            constexpr auto allowedChars = [] {
                std::bitset<256> res{};

                for (char c{'0'}; c <= '1'; c++) res.set(c);
                for (char c{'a'}; c <= 'z'; c++) res.set(c);
                for (char c{'A'}; c <= 'Z'; c++) res.set(c);

                for (char c : "!#$%&\'*+-.^_`|~")
                    res.set(c);

                return res;
            }();

            return allowedChars[c];
        };

        constexpr string_view delimiter { "\r\n" };

        if (headers.ends_with(delimiter))
            headers.remove_suffix(4);

        HttpHeaders res;

        for (const auto& headerAux : headers | vs::split(delimiter)) {
            const string_view header(&*headerAux.begin(), std::ranges::distance(headerAux));
            // conversion needed to minimize copies, headerAux is continuous

            auto separator{ header.find(':') };

            if (header.empty() || separator == string::npos)
                return std::unexpected{ HttpStatusCodeEnum::BAD_REQUEST };

            string_view key{ header };
            string_view val{ header };

            key.remove_suffix(key.size() - separator);
            val.remove_prefix(separator + 1);

            if (!rg::all_of(key, isCharAllowed))
                return std::unexpected{ HttpStatusCodeEnum::BAD_REQUEST };

            const auto startIdx{ val.find_first_not_of(" \t" ) };
            const auto endIdx{ val.find_last_not_of(" \t" ) };

            if (endIdx != string::npos) val.remove_suffix(val.size() - endIdx - 1);
            if (startIdx != string::npos) val.remove_prefix(startIdx);

            if (val.empty() || val[0] == ' ' || val[0] == '\t') // just whitespaces
                return std::unexpected{ HttpStatusCodeEnum::BAD_REQUEST };

            res.Add(key | HeaderSanitizeStr, val);
        }


        return res;
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

                it->second = vs::concat(it->second, ", ", p.second);
#else
                it->second += ", ";
                it->second += p.second;
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

    bool HttpHeaders::Empty() const { return _headers.empty(); }



    HttpHeaders::HeaderValue& HttpHeaders::operator[](HeaderKeyRef key) {
        const auto newKey{ key | HeaderSanitizeStr };
        if (const auto it{  rg::find(_headers, newKey, &HeaderPair::first) }; it != _headers.end())
            return it->second;

        _headers.emplace_back(newKey, string{});
        return _headers.back().second;
    }

    bool HttpHeaders::operator==(const HttpHeaders& other) const = default;
}

#include <Thoth/Http/Headers.hpp>

#include <algorithm>
#include <functional>
#include <ranges>


namespace Thoth::Http {
    using std::string_view;
    using std::string;
    namespace rg = std::ranges;
    namespace vs = std::views;

#pragma region Util
    constexpr int I_ToLower(char c) {
        if ('A' <= c && c <= 'Z')
            return c - 'A' + 'a';
        return c;
    }

    bool I_InsensitiveCmp(const string_view elem1, const string_view elem2) {
        return rg::equal(elem1, elem2,
                [](char a, char b) { return I_ToLower(a) == I_ToLower(b); }
            );
    }

    bool I_UseSemicolon(string_view key) {
        return I_InsensitiveCmp(key, string_view{"cookie"});
    }

    bool I_IsSingleValue(string_view key) {
        constexpr string_view values[] {
            // Date/Time Headers
            "date",
            "expires",
            "last-modified",
            "if-modified-since",
            "if-unmodified-since",

            // Numeric Headers
            "age",
            "content-length",
            "max-forwards",

            // Location/Redirect Headers
            "location",
            "refresh",

            // Entity Headers
            "etag",
            "server",

            // Authorization
            "authorization",
            "proxy-authorization"
        };

        return rg::any_of(values, std::bind_front(I_InsensitiveCmp, key));
    }
    bool I_CanMerge(string_view key) {
        constexpr string_view values[] {
            "set-cookie",
            "www-authenticate",
            "proxy-authenticate"
        };

        return rg::none_of(values, std::bind_front(I_InsensitiveCmp, key));
    }


    template<rg::input_range R, typename T>
    [[nodiscard]] constexpr auto I_FindInsensitiveKey(R&& r, const T& key) {
        const string_view keySv{ key };

        const auto pred = [&](auto&& element) -> bool {
            const string_view elemKeySv{ element.first };

            return I_InsensitiveCmp(elemKeySv, keySv);
        };

        return rg::find_if(std::forward<R>(r), pred);
    }

    template<rg::input_range R, typename T>
    [[nodiscard]] constexpr auto I_FindInsensitiveKeyWithPair(R&& r, const T& p) {
        const string_view keySv{ p.first };
        const string_view valueSv{ p.second };

        const auto pred = [&](auto&& element) -> bool {
            const string_view elemKeySv{ element.first };
            const string_view elemValueSv{ element.second };

            return I_InsensitiveCmp(keySv, elemKeySv) && rg::equal(valueSv, elemValueSv);
        };

        return rg::find_if(std::forward<R>(r), pred);
    }

    constexpr auto inline I_HeaderSanitizeStr = vs::transform(I_ToLower) | rg::to<string>();
#pragma endregion


    Headers::Headers() = default;

    Headers::Headers(const MapType& initAs) {
        _headers.reserve(initAs.size());

        for (const auto& [key, val] : initAs)
            _headers.emplace_back(key | I_HeaderSanitizeStr, val);
    }

    Headers::Headers(const std::initializer_list<HeaderPair> &init) {
        _headers.reserve(init.size());

        for (const auto& [key, val] : init)
            _headers.emplace_back(key | I_HeaderSanitizeStr, val);
    }

    Headers Headers::DefaultHeaders() {
        return {
            {"accept", "*/*" },
            {"user-agent", "Thoth/0.1" },
            {"accept encoding", "identity" }
        };
    }



    bool Headers::Exists(const HeaderKeyRef key) const {
        return I_FindInsensitiveKey(_headers, key) != _headers.end();
    }

    bool Headers::Exists(const HeaderPairRef p) const {
        return I_FindInsensitiveKeyWithPair(_headers, p) != _headers.end();
    }

    bool Headers::Exists(const HeaderKeyRef key, const HeaderValueRef val) const {
        return Exists({key, val});
    }



    void Headers::Add(const HeaderPairRef p) {
        if (I_IsSingleValue(p.first)) {
            Set(p);
            return;
        }

        string_view sep{ I_UseSemicolon(p.first) ? "; " : ", " };

        if (I_CanMerge(p.first) )
            if (const auto it{ I_FindInsensitiveKey(_headers, p.first) }; it != _headers.end()) {
#ifdef __cpp_lib_ranges_concat
                it->second = vs::concat(it->second, sep, p.second);
#else
                it->second += sep;
                it->second += p.second;
#endif
                return;
            }


        _headers.emplace_back(p.first| I_HeaderSanitizeStr, p.second);
    }

    void Headers::Add(const HeaderKeyRef key, const HeaderValueRef val) {
        Add({key, val});
    }

    void Headers::Set(const HeaderPairRef p) {
        std::erase_if(_headers, [&](const HeaderPair& current) {
            return I_InsensitiveCmp(current.first, p.first);
        });

        _headers.emplace_back(p.first | I_HeaderSanitizeStr, p.second);
    }

    void Headers::Set(const HeaderKeyRef key, const HeaderValueRef val) {
        Set({key, val});
    }

    bool Headers::Remove(const HeaderPairRef p) {
        auto&& it{ I_FindInsensitiveKeyWithPair(_headers,  p) };
        if (it == _headers.end())
            return false;

        _headers.erase(it);

        return true;
    }

    bool Headers::Remove(const HeaderKeyRef key, const HeaderValueRef val) {
        return Remove({key, val});
    }

    bool Headers::SetIfNull(HeaderPairRef p) {
        if (Exists(p.first))
            return false;

        Set(p);
        return true;
    }

    bool Headers::SetIfNull(const HeaderKeyRef key, const HeaderValueRef val) {
        return SetIfNull({key, val});
    }



    std::optional<std::reference_wrapper<Headers::HeaderValue>> Headers::Get(HeaderKeyRef key) {
        const auto it{  I_FindInsensitiveKey(_headers, key) };

        if (it != _headers.end())
            return it->second;

        return std::nullopt;
    }

    std::optional<std::reference_wrapper<const Headers::HeaderValue>> Headers::Get(HeaderKeyRef key) const {
        const auto it{  I_FindInsensitiveKey(_headers, key) };

        if (it != _headers.end())
            return it->second;

        return std::nullopt;
    }



    std::vector<Headers::HeaderValue> Headers::GetSetCookie() const {
        return GetSetCookieView() | rg::to<std::vector>();
    }


    Headers::IterType Headers::begin() { return _headers.begin(); }

    Headers::IterType Headers::end() { return _headers.end(); }

    Headers::CIterType Headers::begin() const { return _headers.cbegin(); }

    Headers::CIterType Headers::end() const { return _headers.cend(); }

    Headers::RIterType Headers::rbegin() { return _headers.rbegin(); }

    Headers::RIterType Headers::rend() { return _headers.rend(); }

    Headers::CRIterType Headers::rbegin() const { return _headers.crbegin(); }

    Headers::CRIterType Headers::rend() const { return _headers.crend(); }



    void Headers::Clear() { _headers.clear(); }

    size_t Headers::Size() const { return _headers.size(); }

    bool Headers::Empty() const { return _headers.empty(); }



    Headers::HeaderValue& Headers::operator[](HeaderKeyRef key) {
        if (const auto it{  I_FindInsensitiveKey(_headers, key) }; it != _headers.end())
            return it->second;

        _headers.emplace_back(key | I_HeaderSanitizeStr, string{});
        return _headers.back().second;
    }

    bool Headers::operator==(const Headers& other) const = default;
}

#include <Thoth/Http/NHeaders/Headers.hpp>

#include <algorithm>
#include <functional>
#include <ranges>


namespace Thoth::Http {
    using std::string_view;
    using std::string;
    namespace rg = std::ranges;
    namespace vs = std::views;

#pragma region Util
    constexpr int ToLower(char c) {
        if ('A' <= c && c <= 'Z')
            return c - 'A' + 'a';
        return c;
    }

    bool InsensitiveCmp(const std::string_view elem1, const std::string_view elem2) {
        return rg::equal(elem1, elem2,
                [](char a, char b) { return ToLower(a) == ToLower(b); }
            );
    }

    bool UseSemicolon(std::string_view key) {
        return InsensitiveCmp(key, std::string_view{"cookie"});
    }

    bool IsSingleValue(std::string_view key) {
        constexpr std::string_view values[] {
            // Date/Time Headers
            "date",
            "expires",
            "last-modified",
            "if-modified-since",
            "if-unmodified-since",
            "if-range",
            "retry-after",

            // Numeric Headers
            "age",
            "content-length",
            "max-forwards",

            // Location/Redirect Headers
            "location",
            "refresh",
            "content-location",

            // Entity Headers
            "etag",
            "server",
            "content-type",
            "host",
            "origin",
            "from",

            // Authorization
            "authorization",
            "proxy-authorization"
        };

        return rg::any_of(values, std::bind_front(InsensitiveCmp, key));
    }
    bool CanMerge(std::string_view key) {
        constexpr std::string_view values[] {
            "set-cookie",
            "www-authenticate",
            "proxy-authenticate"
        };

        return rg::none_of(values, std::bind_front(InsensitiveCmp, key));
    }


    template<rg::input_range R, class T>
    [[nodiscard]] constexpr auto FindInsensitiveKey(R&& r, const T& key) {
        const std::string_view keySv{ key };

        const auto pred = [&](auto&& element) -> bool {
            const std::string_view elemKeySv{ element.first };

            return InsensitiveCmp(elemKeySv, keySv);
        };

        return rg::find_if(std::forward<R>(r), pred);
    }

    template<rg::input_range R, class T>
    [[nodiscard]] constexpr auto FindInsensitiveKeyWithPair(R&& r, const T& p) {
        const std::string_view keySv{ p.first };
        const std::string_view valueSv{ p.second };

        const auto pred = [&](auto&& element) -> bool {
            const std::string_view elemKeySv{ element.first };
            const std::string_view elemValueSv{ element.second };

            return InsensitiveCmp(keySv, elemKeySv) && rg::equal(valueSv, elemValueSv);
        };

        return rg::find_if(std::forward<R>(r), pred);
    }

    constexpr auto inline headerSanitizeStr = vs::transform(ToLower) | rg::to<std::string>();
#pragma endregion


    Headers::Headers() = default;

    Headers::Headers(const MapType& initAs) {
        m_headers.reserve(initAs.size());

        for (const auto& [key, val] : initAs)
            m_headers.emplace_back(key | headerSanitizeStr, val);
    }

    Headers::Headers(std::initializer_list<HeaderPair> init) {
        m_headers.reserve(init.size());

        for (const auto& [key, val] : init)
            m_headers.emplace_back(key | headerSanitizeStr, val);
    }

    Headers Headers::DefaultHeaders() {
        return {
            {"accept", "*/*" },
            {"user-agent", "Thoth/0.1" },
            {"accept-encoding", "identity" }
        };
    }



    bool Headers::Exists(const HeaderKeyRef key) const {
        return FindInsensitiveKey(m_headers, key) != m_headers.end();
    }

    bool Headers::Exists(const HeaderPairRef p) const {
        return FindInsensitiveKeyWithPair(m_headers, p) != m_headers.end();
    }

    bool Headers::Exists(const HeaderKeyRef key, const HeaderValueRef val) const {
        return Exists({key, val});
    }



    void Headers::Add(const HeaderPairRef p) {
        if (IsSingleValue(p.first)) {
            Set(p);
            return;
        }

        std::string_view sep{ UseSemicolon(p.first) ? "; " : ", " };

        if (CanMerge(p.first) )
            if (const auto it{ FindInsensitiveKey(m_headers, p.first) }; it != m_headers.end()) {
#ifdef __cpp_lib_ranges_concat
                it->second.assign_range(vs::concat(it->second, sep, p.second));
#else
                it->second += sep;
                it->second += p.second;
#endif
                return;
            }


        m_headers.emplace_back(p.first| headerSanitizeStr, p.second);
    }

    void Headers::Add(const HeaderKeyRef key, const HeaderValueRef val) {
        Add({key, val});
    }

    void Headers::Set(const HeaderPairRef p) {
        std::erase_if(m_headers, [&](const HeaderPair& current) {
            return InsensitiveCmp(current.first, p.first);
        });

        m_headers.emplace_back(p.first | headerSanitizeStr, p.second);
    }

    void Headers::Set(const HeaderKeyRef key, const HeaderValueRef val) {
        Set({key, val});
    }


    bool Headers::Remove(HeaderKeyRef k) {
        auto&& it{ FindInsensitiveKey(m_headers, k) };
        if (it == m_headers.end())
            return false;

        m_headers.erase(it);

        return true;
    }

    bool Headers::Remove(const HeaderPairRef p) {
        auto&& it{ FindInsensitiveKeyWithPair(m_headers, p) };
        if (it == m_headers.end())
            return false;

        m_headers.erase(it);

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



    std::optional<Headers::HeaderValue*> Headers::Get(HeaderKeyRef key) {
        const auto it{  FindInsensitiveKey(m_headers, key) };

        if (it != m_headers.end())
            return &it->second;

        return std::nullopt;
    }

    std::optional<const Headers::HeaderValue*> Headers::Get(HeaderKeyRef key) const {
        const auto it{  FindInsensitiveKey(m_headers, key) };

        if (it != m_headers.end())
            return &it->second;

        return std::nullopt;
    }

    NHeaders::ListProxy<false, NHeaders::MimeType> Headers::Accept() {
        return { "accept", *this };
    }

    NHeaders::ListProxy<true, NHeaders::MimeType> Headers::Accept() const {
        return { "accept", *this };
    }


    NHeaders::ListProxy<false, NHeaders::AcceptEncodingEnum> Headers::AcceptEncoding() {
        return { "accept-encoding", *this };
    }

    NHeaders::ListProxy<true, NHeaders::AcceptEncodingEnum> Headers::AcceptEncoding() const {
        return { "accept-encoding", *this };
    }


    NHeaders::ValueProxy<false, NHeaders::MimeType> Headers::ContentType() {
        return { "content-type", *this };
    }

    NHeaders::ValueProxy<true, NHeaders::MimeType> Headers::ContentType() const {
        return { "content-type", *this };
    }


    NHeaders::ValueProxy<false, uint64_t> Headers::ContentLength() {
        return { "content-length", *this };
    }

    NHeaders::ValueProxy<true, uint64_t> Headers::ContentLength() const {
        return { "content-length", *this };
    }


    NHeaders::ListProxy<false, NHeaders::ContentEncodingEnum> Headers::ContentEncoding() {
        return { "content-encoding", *this };
    }

    NHeaders::ListProxy<true, NHeaders::ContentEncodingEnum> Headers::ContentEncoding() const {
        return { "content-encoding", *this };
    }


    NHeaders::ListProxy<false, NHeaders::TransferEncodingEnum> Headers::TransferEncoding() {
        return { "transfer-encoding", *this };
    }

    NHeaders::ListProxy<true, NHeaders::TransferEncodingEnum> Headers::TransferEncoding() const {
        return { "transfer-encoding", *this };
    }


    NHeaders::ListProxy<false, std::string> Headers::ContentLanguage() {
        return { "content-language", *this };
    }

    NHeaders::ListProxy<true, std::string> Headers::ContentLanguage() const {
        return { "content-language", *this };
    }


    NHeaders::ValueProxy<false, std::string> Headers::ContentLocation() {
        return { "content-location", *this };
    }

    NHeaders::ValueProxy<true, std::string> Headers::ContentLocation() const {
        return { "content-location", *this };
    }


    NHeaders::ValueProxy<false, std::chrono::utc_clock::time_point> Headers::Date() {
        return { "date", *this, "%a, %d %b %Y %H:%M:%S GMT" };
    }

    NHeaders::ValueProxy<true, std::chrono::utc_clock::time_point> Headers::Date() const {
        return { "date", *this, "%a, %d %b %Y %H:%M:%S GMT" };
    }


    NHeaders::ListProxy<false, std::string> Headers::Connection() {
        return { "connection", *this };
    }

    NHeaders::ListProxy<true, std::string> Headers::Connection() const {
        return { "connection", *this };
    }


    NHeaders::ListProxy<false, NHeaders::Upgrade> Headers::Upgrade() {
        return { "upgrade", *this };
    }

    NHeaders::ListProxy<true, NHeaders::Upgrade> Headers::Upgrade() const {
        return { "upgrade", *this };
    }


    NHeaders::ListProxy<false, std::string> Headers::Trailer() {
        return { "trailer", *this };
    }

    NHeaders::ListProxy<true, std::string> Headers::Trailer() const {
        return { "trailer", *this };
    }


    NHeaders::ListProxy<false, std::string> Headers::Via() {
        return { "via", *this };
    }

    NHeaders::ListProxy<true, std::string> Headers::Via() const {
        return { "via", *this };
    }




    // std::vector<Headers::HeaderValue> Headers::GetSetCookie() const {
    //     return GetSetCookieView() | rg::to<std::vector>();
    // }
    //




    Headers::IterType Headers::begin() { return m_headers.begin(); }

    Headers::IterType Headers::end() { return m_headers.end(); }

    Headers::CIterType Headers::begin() const { return m_headers.cbegin(); }

    Headers::CIterType Headers::end() const { return m_headers.cend(); }

    Headers::RIterType Headers::rbegin() { return m_headers.rbegin(); }

    Headers::RIterType Headers::rend() { return m_headers.rend(); }

    Headers::CRIterType Headers::rbegin() const { return m_headers.crbegin(); }

    Headers::CRIterType Headers::rend() const { return m_headers.crend(); }



    void Headers::Clear() { m_headers.clear(); }

    size_t Headers::Size() const { return m_headers.size(); }

    bool Headers::Empty() const { return m_headers.empty(); }



    Headers::HeaderValue& Headers::operator[](HeaderKeyRef key) {
        if (const auto it{  FindInsensitiveKey(m_headers, key) }; it != m_headers.end())
            return it->second;

        m_headers.emplace_back(key | headerSanitizeStr, string{});
        return m_headers.back().second;
    }

    bool Headers::operator==(const Headers& other) const {
        static constexpr auto headerEqual{ [](const auto &a, const auto &b) {
            return std::ranges::equal(a.first, b.first, String::CaseInsensitiveCompare) && a.second == b.second;
        } };

        return std::ranges::is_permutation(m_headers, other.m_headers, headerEqual);

    }
}

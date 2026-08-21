#include <Thoth/Http/Url/Url.hpp>
#include <algorithm>

#include <Thoth/String/Utils.hpp>

#pragma push_macro("FAIL_WITH")
#undef FAIL_WITH

#define FAIL_WITH(x) return ThothUnex{ UrlParseErrorEnum::x }


using Thoth::Http::Url;
using std::string_view;
using std::string;

namespace rg = std::ranges;
namespace vs = std::views;


namespace {
    // RFC 3986 §5.2.4 — Remove Dot Segments
    std::string RemoveDotSegments(std::string_view path) {
        std::string out;
        out.reserve(path.size());

        while (!path.empty()) {
            // A: strip leading "../" or "./"
            if      (path.starts_with("../")) { path.remove_prefix(3); }
            else if (path.starts_with("./" )) { path.remove_prefix(2); }
            // B: collapse "/./" → "/"
            else if (path.starts_with("/./")) { path.remove_prefix(2); }
            else if (path == "/."           ) { path = "/";            }
            // C: collapse "/../" → "/" and pop last output segment
            else if (path.starts_with("/../")) {
                path.remove_prefix(3);
                if (const auto p{ out.rfind('/') }; p != std::string::npos)
                    out.erase(p);
            }
            else if (path == "/.." ) {
                path = "/";
                if (const auto p{ out.rfind('/') }; p != std::string::npos)
                    out.erase(p);
            }
            // D: lone "." or ".." — discard
            else if (path == "." || path == "..") { break; }
            // E: move first segment (including leading "/" if any) to output
            else {
                const auto segEnd{ path.find('/', path[0] == '/' ? 1 : 0) };
                const auto take  { segEnd == std::string_view::npos ? path.size() : segEnd };
                out  += path.substr(0, take);
                path.remove_prefix(take);
            }
        }
        return out;
    }

    struct ParsedRef {
        std::string_view scheme{};
        std::string_view authority{};
        std::string_view path{};
        std::string_view query{};
        std::string_view fragment{};
        bool hasScheme{};
        bool hasAuthority{};
        bool hasQuery{};
        bool hasFragment{};
    };

    // Lightweight reference parser — does NOT validate, just splits components.
    ParsedRef ParseRef(std::string_view ref) {
        ParsedRef r;
        std::string_view rest{ ref };

        // Fragment — must come first so '?' inside fragment is not misread as query
        if (const auto pos{ rest.find('#') }; pos != std::string_view::npos) {
            r.fragment    = rest.substr(pos + 1);
            r.hasFragment = true;
            rest.remove_suffix(rest.size() - pos);
        }

        // Query
        if (const auto pos{ rest.find('?') }; pos != std::string_view::npos) {
            r.query    = rest.substr(pos + 1);
            r.hasQuery = true;
            rest.remove_suffix(rest.size() - pos);
        }

        // Scheme — ALPHA *( ALPHA / DIGIT / "+" / "-" / "." ) ":"
        {
            std::size_t i{};
            bool valid{ !rest.empty() && std::isalpha(static_cast<unsigned char>(rest[0])) };
            while (valid && i < rest.size() && rest[i] != ':' && rest[i] != '/') {
                const char c{ rest[i++] };
                valid = std::isalnum(static_cast<unsigned char>(c))
                     || c == '+' || c == '-' || c == '.';
            }
            if (valid && i < rest.size() && rest[i] == ':') {
                r.scheme    = rest.substr(0, i);
                r.hasScheme = true;
                rest.remove_prefix(i + 1);
            }
        }

        // Authority
        if (rest.size() >= 2 && rest[0] == '/' && rest[1] == '/') {
            rest.remove_prefix(2);
            const auto pos{ rest.find('/') };
            r.authority    = rest.substr(0, pos);
            r.hasAuthority = true;
            rest = (pos != std::string_view::npos) ? rest.substr(pos) : std::string_view{};
        }

        r.path = rest;
        return r;
    }
} // anonymous namespace


std::string Thoth::Http::Authority::GetHostString() const {
    using Hermes::IpAddress;

    return std::visit(Hermes::Utils::Overloaded{
        [](string    str){ return std::move(str); },
        [](IpAddress ip ){ return std::format("{}", ip); }
    }, host);
}

std::string Thoth::Http::AuthorityView::GetHostString() const {
    using Hermes::IpAddress;

    return std::visit(Hermes::Utils::Overloaded{
        [](string_view str){ return std::string{ str }; },
        [](IpAddress   ip ){ return std::format("{}", ip); }
    }, host);
}

std::optional<std::uint16_t> Thoth::Http::GetDefaultPort(const std::string_view scheme) noexcept {
    if (scheme == "http"  ) return 80;
    if (scheme == "https" ) return 443;
    if (scheme == "ws"    ) return 80;
    if (scheme == "wss"   ) return 443;
    if (scheme == "ftp"   ) return 21;
    if (scheme == "gemini") return 1965; // honorable mention

    return std::nullopt;
}








std::string_view Url::View(const RangeIdx range) const noexcept {
    const auto [l, r] { range };
    return string_view{ m_rawUrl }.substr(l, r);
}

Url::RangeIdx Url::MakeRange(const std::string& source, const std::string_view view) noexcept {
    if (view.data() == nullptr)
        return {};

    return {
        static_cast<std::size_t>(view.data() - source.data()),
        view.size()
    };
}

std::string_view Url::GetScheme() const noexcept {
    return View(m_schemeIdx);
}
Url::AuthorityViewOpt Url::GetAuthority() const noexcept {
    if (!m_authority)
        return std::nullopt;

    AuthorityView result;
    result.userinfo = View(m_authority->userinfo);
    result.port     = m_authority->port;
    result.host     = std::visit([this](const auto& host) -> HostView {
        using T = std::decay_t<decltype(host)>;
        if constexpr (std::same_as<T, RangeIdx>)
            return View(host);
        else
            return host;
    }, m_authority->host);

    return result;
}
std::string_view Url::GetPath() const noexcept {
    return View(m_pathIdx);
}
std::string_view Url::GetQuery() const noexcept {
    return m_queryIdx ? View(*m_queryIdx) : std::string_view{};
}
std::string_view Url::GetFragment() const noexcept {
    return m_fragIdx ? View(*m_fragIdx) : std::string_view{};
}

std::string Url::CopyScheme() const noexcept {
    const auto [l, r] { m_schemeIdx };
    return m_rawUrl.substr(l, r);
}

Url::AuthorityOpt Url::CopyAuthority() const noexcept {
    if (!m_authority)
        return std::nullopt;

    const auto port{ m_authority->port };
    auto uInfo{ std::string{ View(m_authority->userinfo) } };

    return std::visit(Hermes::Utils::Overloaded{
        [this, &uInfo, port](RangeIdx range) { return Authority{ uInfo, { std::string{ View(range) } }, port }; },
        [&uInfo, port](Hermes::IpAddress val) { return Authority{ uInfo, val, port }; }
    }, m_authority->host);
}

std::string Url::CopyPath() const noexcept {
    const auto [l, r] { m_pathIdx };
    return m_rawUrl.substr(l, r);
}

std::string Url::CopyQuery() const noexcept {
    return m_queryIdx ? std::string{ View(*m_queryIdx) } : std::string{};
}

std::string Url::CopyFragment() const noexcept {
    return m_fragIdx ? std::string{ View(*m_fragIdx) } : std::string{};
}


std::string_view Url::GetPathOrSep() const noexcept { return GetPath().empty() ? "/" : GetPath(); }

Thoth::Http::QueryParams Url::GetQueryParams() const { return QueryParams::Parse(GetQuery()); }

std::string_view Url::GetUrlWithoutFragment() const noexcept {
    return string_view{ m_rawUrl }
    .substr(0, m_fragIdx
            ? m_fragIdx->first - 1
            : std::string::npos);
}



// Normally I would do this in a functional way but... Aff, there are to many rules.

// URL parsing from RFC3986


std::expected<Url, Thoth::ThothError> Url::FromUrl(std::string rawUrl) {
    if (rawUrl.empty() || !isalpha(rawUrl.front()))
        FAIL_WITH(EmptyUrl);

    Url url{};
    url.m_rawUrl = std::move(rawUrl);
    std::string_view rawUrlView{ url.m_rawUrl };
    std::string_view scheme;
    AuthorityView    auth;
    std::string_view hostStr;
    std::string_view path;
    std::string_view query;
    std::string_view frag;

    std::string_view hierPart;

#pragma region General

    // URI         = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
    //
    //      hier-part   = "//" authority path-abempty
    //                  / path-absolute
    //                  / path-rootless
    //                  / path-empty

    // * Just `hier-part = authority path-abempty` is implemented

    // TODO: reinforce the scheme check
    // if (!m_rawUrl.starts_with("http:") && !m_rawUrl.starts_with("https:")) // its URL after all
    //     FAIL_WITH(InvalidScheme); // ill-formed, scheme is mandatory

    const auto schemeIdx{ rawUrlView.find(':') };

    if (schemeIdx == string::npos)
        FAIL_WITH(InvalidScheme); // ill-formed, scheme is mandatory

    scheme = string_view(rawUrlView.data(), schemeIdx);

    static constexpr auto a_acceptedSchemeChars = [](char c) {
        constexpr auto bitset{ String::MakeBitset({ String::CharSequences::k_alphanumeric, "+-." }) };
        return bitset[c];
    };
    if (scheme.empty() || !isalpha(scheme[0]) || !rg::all_of(scheme, a_acceptedSchemeChars))
        FAIL_WITH(InvalidScheme);

    rawUrlView.remove_prefix(schemeIdx + 1);


    if (const auto hierPartIdx{ rawUrlView.find_first_of("?#") }; hierPartIdx == string::npos)
        hierPart = rawUrlView; // no query or fragment
    else {
        hierPart = string_view(rawUrlView.data(), hierPartIdx);
        char hierPartDelimiter{ rawUrlView[hierPartIdx] };

        rawUrlView.remove_prefix(hierPartIdx + 1);

        if (hierPartDelimiter == '#')
            frag = rawUrlView; // no query, just fragment
        else {
            auto it{ rg::find(rawUrlView, '#') }; // a query and maybe a fragment too

            query = string_view(rawUrlView.begin(), it);

            if (it != rawUrlView.end())
                frag = string_view(it + 1, rawUrlView.end());
        }
    }

#pragma endregion
#pragma region Scheme

    // scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )

#pragma endregion
#pragma region Auth & UserInfo

    // authority   = [ userinfo "@" ] host [ ":" port ]
    // userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )


    if (hierPart.empty())
        FAIL_WITH(IllFormed);

    if (!hierPart.starts_with("//"))
        FAIL_WITH(IllFormed); // in HTTP, {"//" authority path-abempty} is mandatory

    hierPart.remove_prefix(2);
    auto pathIdx{ hierPart.find('/') };

    if (pathIdx == 0)
        FAIL_WITH(IllFormed); // ill-formed, authority is empty

    if (pathIdx != string::npos) {
        path = string_view{ pathIdx + hierPart.begin(), hierPart.end() }; // includes '/' NOLINT(*-narrowing-conversions)

        hierPart.remove_suffix(path.size());
    }


    auto authIdx{ static_cast<long long>(hierPart.find('@')) };

    auth.userinfo = string_view(hierPart.data(), std::max(0LL, authIdx));
    hierPart.remove_prefix(authIdx + 1);

    // TODO: check chars from userinfo

    size_t portIdx;
    if (hierPart.starts_with('[')) {
        const auto closeBracket{ hierPart.find(']') };
        if (closeBracket == string_view::npos)
            FAIL_WITH(IllFormed);
        portIdx = hierPart.find(':', closeBracket + 1);
    } else {
        portIdx = hierPart.find(':');
    }

    if (portIdx == string_view::npos) // no port
        hostStr = hierPart;
    else {
        hostStr = string_view(hierPart.data(), portIdx);

        // hierPart.rbegin() + 1 is hierPart.end(), but I cant dereference it in MSVC

        auth.port = 0; // placing a value
        auto [ptr, ec] = std::from_chars(&*hierPart.begin() + portIdx + 1, &*hierPart.rbegin() + 1, *auth.port);

        if (ec != std::errc() || ptr != &*hierPart.rbegin() + 1)
            FAIL_WITH(InvalidPort);

        hierPart.remove_prefix(portIdx + 1);
    }

#pragma endregion
#pragma region Host

    if (hostStr.empty())
        FAIL_WITH(IllFormed); // ill-formed, in auth host is mandatory

    // (omg there is so much thing on here. Ok, fuck IPs, I will do just reg-name for now)

    // host        = IP-literal / IPv4address / reg-name
    // IP-literal = "[" ( IPv6address / IPvFuture  ) "]"
    // IPvFuture  = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )

    // IPv6address =                            6( h16 ":" ) ls32
    //             /                       "::" 5( h16 ":" ) ls32
    //             / [               h16 ] "::" 4( h16 ":" ) ls32
    //             / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
    //             / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
    //             / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
    //             / [ *4( h16 ":" ) h16 ] "::"              ls32
    //             / [ *5( h16 ":" ) h16 ] "::"              h16
    //             / [ *6( h16 ":" ) h16 ] "::"
    //
    // ls32        = ( h16 ":" h16 ) / IPv4address
    //             ; least-significant 32 bits of address
    //
    // h16         = 1*4HEXDIG
    //             ; 16 bits of address represented in hexadecimal

    const auto readIpv6 = [hostStr]() -> std::optional<HostView> {
        if (!hostStr.starts_with('[') || !hostStr.ends_with(']'))
            return std::nullopt;

        const std::string inner{ hostStr.substr(1, hostStr.size() - 2) };
        auto ip{ Hermes::IpAddress::TryParse(inner) };

        if (!ip || !ip->IsIpv6())
            return std::nullopt;

        return HostView{ *ip };
    };

    // IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet
    // dec-octet   = DIGIT                 ; 0-9
    //             / %x31-39 DIGIT         ; 10-99
    //             / "1" 2DIGIT            ; 100-199
    //             / "2" %x30-34 DIGIT     ; 200-249
    //             / "25" %x30-35          ; 250-255

    const auto readIpv4 = [hostStr]() -> std::optional<HostView> {
        Hermes::IpAddress::Ipv4Type ip{};

        const char* ptr{ hostStr.data() };
        const char* end{ hostStr.data() + hostStr.size() };
        for (int i{}; i < 4; ++i) {
            uint8_t val;

            auto [newPtr, err]{ std::from_chars(ptr, hostStr.data() + hostStr.size(), val) };

            if (err != std::errc{})
                return std::nullopt;
            if (i == 3
                    ? newPtr != end
                    : newPtr == end || *newPtr != '.')
                return std::nullopt;
            if (*ptr == '0' && newPtr > ptr + 1)
                return std::nullopt;

            ip[i] = val;
            ptr = newPtr + 1;
        }

        return { Hermes::IpAddress{ ip } };
    };

    // reg-name    = *( unreserved / pct-encoded / sub-delims )

    auto readRegName = [hostStr]() -> std::optional<HostView> {
        constexpr auto set{ String::MakeBitset({ String::CharSequences::Http::k_url }) };

        for (size_t i{}; i < hostStr.size(); ++i) {
            const char c{ hostStr[i] };

            if (set[static_cast<unsigned char>(c)]) continue;


            if (c == '%') {
                if (i + 2 >= hostStr.size() ||
                    !std::isxdigit(hostStr[i + 1]) ||
                    !std::isxdigit(hostStr[i + 2])) {
                    return std::nullopt;
                    }
                i += 2;
                continue;
            }

            return std::nullopt;
        }

        return hostStr;
    };

    if (auto host{ readIpv6().or_else(readIpv4).or_else(readRegName) }; host)
        auth.host = *host;
    else
        FAIL_WITH(HostIsRequired);


#pragma endregion
#pragma region Port

    // port        = *DIGIT


#pragma endregion
#pragma region Path

    // path          = path-abempty    ; begins with "/" or is empty
    //               / path-absolute   ; begins with "/" but not "//"
    //               / path-noscheme   ; begins with a non-colon segment
    //               / path-rootless   ; begins with a segment
    //               / path-empty      ; zero characters
    //
    // path-abempty  = *( "/" segment )
    // path-absolute = "/" [ segment-nz *( "/" segment ) ]
    // path-noscheme = segment-nz-nc *( "/" segment )
    // path-rootless = segment-nz *( "/" segment )
    // path-empty    = 0<pchar>
    //
    //
    // segment       = *pchar
    // segment-nz    = 1*pchar
    // segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
    //               ; non-zero-length segment without any colon ":"
    //
    // pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"

#pragma endregion
#pragma region Query

    // query       = *( pchar / "/" / "?" )

#pragma endregion
#pragma region Fragment

    // fragment    = *( pchar / "/" / "?" )

#pragma endregion





    Url::AuthorityIdx authority{};
    authority.userinfo = MakeRange(url.m_rawUrl, auth.userinfo);
    authority.host = std::visit([&](const auto& host) -> std::variant<Url::RangeIdx, Hermes::IpAddress> {
        using T = std::decay_t<decltype(host)>;
        if constexpr (std::same_as<T, std::string_view>)
            return MakeRange(url.m_rawUrl, host);
        else
            return host;
    }, auth.host);
    authority.port = auth.port;

    url.m_authority = std::move(authority);
    url.m_schemeIdx = MakeRange(url.m_rawUrl, scheme);
    url.m_pathIdx   = MakeRange(url.m_rawUrl, path);
    if (query.data() != nullptr)
        url.m_queryIdx = MakeRange(url.m_rawUrl, query);
    if (frag.data() != nullptr)
        url.m_fragIdx = MakeRange(url.m_rawUrl, frag);

    return url;
}

std::expected<Url, Thoth::ThothError> Url::Resolve(std::string_view reference) const {
    // RFC 3986 §5.2.2
    const ParsedRef r{ ParseRef(reference) };

    std::string tScheme, tAuthority, tPath, tQuery, tFragment;
    bool hasAuthority{}, hasQuery{};

    // Reconstruct the base authority string (userinfo@host:port).
    const auto baseAuthority{ [&]() -> std::string {
    const auto auth{ GetAuthority() };
    if (!auth) return {};

    std::string s;
    if (!auth->userinfo.empty()) { s += auth->userinfo; s += '@'; }

    // URL authority requer brackets em IPv6 (RFC 3986 §3.2.2: IP-literal = "[" IPv6address "]")
    // Não usar GetHostString() aqui — ele omite os brackets intencionalmente para uso em SNI/DNS.
    s += std::visit([]<class T>(T host) -> std::string {
        if constexpr (std::same_as<T, std::string_view>)
            return std::string{ host };
        else
            return std::format("{:b}", host); // :b = com brackets
    }, auth->host);

    if (auth->port) { s += ':'; s += std::to_string(*auth->port); }
    return s;
    } };

    if (r.hasScheme) {
        // Absolute reference — use R directly.
        tScheme      = r.scheme;
        hasAuthority = r.hasAuthority;
        tAuthority   = r.authority;
        tPath        = RemoveDotSegments(r.path);
        hasQuery     = r.hasQuery;
        tQuery       = r.query;
    } else {
        if (r.hasAuthority) {
            hasAuthority = true;
            tAuthority   = r.authority;
            tPath        = RemoveDotSegments(r.path);
            hasQuery     = r.hasQuery;
            tQuery       = r.query;
        } else {
            if (r.path.empty()) {
                // Same-document or fragment-only reference — keep base path.
                tPath = GetPath();
                if (r.hasQuery) {
                    hasQuery = true;
                    tQuery   = r.query;
                } else {
                    // Preserve base query only if non-empty.
                    const auto bq{ GetQuery() };
                    if (!bq.empty()) { hasQuery = true; tQuery = bq; }
                }
            } else {
                if (r.path.starts_with('/')) {
                    tPath = RemoveDotSegments(r.path);
                } else {
                    // Merge: base path up to and including last '/' + R path.
                    const auto basePath{ GetPath() };
                    std::string merged;

                    if (GetAuthority() && basePath.empty()) {
                        merged  = '/';
                        merged += r.path;
                    } else {
                        const auto slash{ basePath.rfind('/') };
                        if (slash != std::string_view::npos)
                            merged = std::string{ basePath.substr(0, slash + 1) };
                        merged += r.path;
                    }
                    tPath = RemoveDotSegments(merged);
                }
                hasQuery = r.hasQuery;
                tQuery   = r.query;
            }

            // Inherit base authority.
            const auto ba{ baseAuthority() };
            hasAuthority = !ba.empty();
            tAuthority   = ba;
        }
        tScheme = GetScheme();
    }

    tFragment = r.fragment;

    // Reconstruct absolute URL string.
    std::string target;
    target.reserve(tScheme.size() + 3 + tAuthority.size()
                 + tPath.size() + tQuery.size() + tFragment.size() + 4);
    target += tScheme;
    target += ':';
    if (hasAuthority)                 { target += "//"; target += tAuthority; }
    target                            += tPath;
    if (hasQuery)                     { target += '?';  target += tQuery;     }
    if (r.hasFragment)                { target += '#';  target += tFragment;  }

    return Url::FromUrl(std::move(target));
}

std::expected<Url, Thoth::ThothError> Url::ResolveRelative(const Url& url, std::string_view reference) {
    return url.Resolve(reference);
}

std::string Url::Encode(std::string_view str) {
    string buffer;
    buffer.reserve(3 * str.size());

    for (const unsigned char c : str) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            buffer += static_cast<char>(c);
        else
            std::format_to(std::back_inserter(buffer),"%{:02X}", c);
    }
    return buffer;
}


constexpr auto hexCharToInt = [] {
    std::array<int, 256> toHex{};
    for (char c{'0'}; c <= '9'; c++) toHex[c] = c - '0';
    for (char c{'a'}; c <= 'z'; c++) toHex[c] = c - 'a' + 10;
    for (char c{'A'}; c <= 'Z'; c++) toHex[c] = c - 'A' + 10;

    return toHex;
}();


std::expected<string, Thoth::ThothError> Url::TryDecode(std::string_view str) {
    std::string buffer;
    buffer.reserve(str.length());


    for (int i{}; i < str.size(); i++) {
        if (str[i] == '%') {
            if (i + 2 >= str.length() || !std::isxdigit(str[i + 1]) || !std::isxdigit(str[i + 2]))
                FAIL_WITH(IllFormed);

            const int high{ hexCharToInt[str[i + 1]] };
            const int low{ hexCharToInt[str[i + 2]] };
            buffer += static_cast<char>((high << 4) + low);

            i += 2;
        } else if (str[i] == '+')
            buffer += ' ';
        else
            buffer += str[i];
    }
    return buffer;
}

bool Url::operator==(const Url& other) const noexcept {
    return m_rawUrl == other.m_rawUrl;
}


#pragma pop_macro("FAIL_WITH")

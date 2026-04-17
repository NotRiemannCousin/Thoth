#include <Thoth/Http/Url/Url.hpp>
#include <algorithm>

#include "Thoth/String/Utils.hpp"

using Thoth::Http::Url;
using std::string_view;
using std::string;

namespace rg = std::ranges;
namespace vs = std::views;




std::string Thoth::Http::Authority::GetHostString() const {
    return std::visit([]<class T>(T host) -> std::string {
            if constexpr (std::same_as<T, std::string_view>)
                return std::string{ host };
            else
                return std::format("{}", Hermes::IpAddress(host));
        }, host);
}

#define FAIL_WITH(x) return std::unexpected{ RequestError{ UrlParseErrorEnum::x } }

std::optional<std::uint16_t> Thoth::Http::GetDefaultPort(const std::string_view scheme) noexcept {
    if (scheme == "http"  ) return 80;
    if (scheme == "https" ) return 443;
    if (scheme == "ws"    ) return 80;
    if (scheme == "wss"   ) return 443;
    if (scheme == "ftp"   ) return 21;
    if (scheme == "gemini") return 1965; // honorable mention

    return std::nullopt;
}


#define REBIND_ALL(OTHER_URL)                                              \
        const auto rebind{ Rebinder(rawUrl, (OTHER_URL)) };                \
                                                                           \
        scheme = rebind(other.scheme);                                     \
        path   = rebind(other.path  );                                     \
        query  = rebind(other.query );                                     \
        frag   = rebind(other.frag  );                                     \
                                                                           \
        authority = other.authority.transform([&](Authority auth) {        \
            auth.userinfo = rebind(auth.userinfo);                         \
                                                                           \
            std::visit([&]<class T>(T& host) {                             \
                if constexpr (std::same_as<std::decay_t<T>, std::string_view>)  \
                    host = rebind(host);                                   \
            }, auth.host);                                                 \
            return auth;                                                   \
        });                                                                \

auto Rebinder(std::string_view url, std::string_view otherUrl) {
    return [=](const std::string_view old) -> std::string_view {
        return std::string_view{
            url.data() + std::distance(otherUrl.data(), old.data()),
            old.size()
        };
    };
}


Url::Url(const Url& other) {
    rawUrl = other.rawUrl;

    REBIND_ALL(other.rawUrl)
}

Url::Url(Url&& other) noexcept {
    rawUrl = std::move(other.rawUrl);

    const std::string_view otherUrl{ rawUrl };

    REBIND_ALL(otherUrl)
}

Url& Url::operator=(const Url& other) {
    rawUrl = other.rawUrl;

    REBIND_ALL(other.rawUrl)

    return *this;
}

Url& Url::operator=(Url&& other) noexcept {
    rawUrl = std::move(other.rawUrl);

    const std::string_view otherUrl{ other.rawUrl };

    REBIND_ALL(otherUrl)

    return *this;
}

#undef REBIND_ALL






std::string_view                      Url::GetScheme()    const noexcept { return scheme;    }
std::optional<Thoth::Http::Authority> Url::GetAuthority() const noexcept { return authority; }
std::string_view                      Url::GetPath()      const noexcept { return path;      }
std::string_view                      Url::GetQuery()     const noexcept { return query;     }
std::string_view                      Url::GetFragment()  const noexcept { return frag;      }




std::string_view Url::GetPathOrSep() const noexcept { return path.empty() ? ":" : path; }

Thoth::Http::QueryParams Url::GetQueryParams() const { return QueryParams::Parse(query); }

std::string_view Url::GetUrlWithoutFragment() const noexcept {
    return rawUrl.substr(frag.empty() ? std::string::npos : std::distance(rawUrl.data(), frag.data()) - 1);
}



// Normally I would do this in a functional way but... Aff, there are to many rules.

// URL parsing from RFC3986


std::expected<Url, Thoth::Http::RequestError> Url::FromUrl(std::string rawUrl) {
    if (rawUrl.empty() || !isalpha(rawUrl.front()))
        FAIL_WITH(EmptyUrl);

    std::string_view rawUrlView{ rawUrl };
    std::string_view scheme;
    Authority auth;
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
    // if (!rawUrl.starts_with("http:") && !rawUrl.starts_with("https:")) // its URL after all
    //     FAIL_WITH(InvalidScheme); // ill-formed, scheme is mandatory

    const auto schemeIdx{ rawUrlView.find(':') };
    scheme = string_view(rawUrlView.data(), schemeIdx);
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

    auto portIdx{ hierPart.find(':') };

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

    const auto s_readIpv6 = [hostStr]() -> std::optional<Host> {
        return std::nullopt;
    };
    // TODO: implement IPv6 someday (not today)

    // IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet
    // dec-octet   = DIGIT                 ; 0-9
    //             / %x31-39 DIGIT         ; 10-99
    //             / "1" 2DIGIT            ; 100-199
    //             / "2" %x30-34 DIGIT     ; 200-249
    //             / "25" %x30-35          ; 250-255

    const auto s_readIpv4 = [hostStr]() -> std::optional<Host> {
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

    auto s_readRegName = [hostStr]() -> std::optional<Host> {
        constexpr auto set{ String::MakeBitset({ String::CharSequences::Http::url }) };

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

    if (auto host{ s_readIpv6().or_else(s_readIpv4).or_else(s_readRegName) }; host)
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







    Url res{};

    res.rawUrl    = std::move(rawUrl);
    res.scheme    = scheme;
    res.authority = auth;
    res.path      = path;
    res.query     = query;
    res.frag      = frag;

    return res;
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


constexpr auto s_hexCharToInt = [] {
    std::array<int, 256> toHex{};
    for (char c{'0'}; c <= '9'; c++) toHex[c] = c - '0';
    for (char c{'a'}; c <= 'z'; c++) toHex[c] = c - 'a' + 10;
    for (char c{'A'}; c <= 'Z'; c++) toHex[c] = c - 'A' + 10;

    return toHex;
}();


std::expected<string, Thoth::Http::RequestError> Url::TryDecode(std::string_view str) {
    std::string buffer;
    buffer.reserve(str.length());


    for (int i{}; i < str.size(); i++) {
        if (str[i] == '%') {
            if (i + 2 >= str.length() || !std::isxdigit(str[i + 1]) || !std::isxdigit(str[i + 2]))
                FAIL_WITH(IllFormed);

            const int high{ s_hexCharToInt[str[i + 1]] };
            const int low{ s_hexCharToInt[str[i + 2]] };
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
    return rawUrl == other.rawUrl;
}

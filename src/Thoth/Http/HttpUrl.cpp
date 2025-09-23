#include <Thoth/Http/HttpUrl.hpp>
#include <algorithm>

using Thoth::Http::HttpUrl;
using std::string_view;
using std::string;

namespace rg = std::ranges;
namespace vs = std::views;


// Normally I would do this in a functional way but... Aff, there are to many rules.

// URL parsing from RFC3986

std::optional<HttpUrl> HttpUrl::FromUrl(string_view rawUrl) {

    if (rawUrl.empty() || !isalpha(rawUrl.front())) return std::nullopt;

    string_view scheme;
    string_view user;
    string_view host;
    string_view path;
    int port{};
    string_view query;
    string_view fragment;

    string_view hierPart;

#pragma region General

    // URI         = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
    //
    //      hier-part   = "//" authority path-abempty
    //                  / path-absolute
    //                  / path-rootless
    //                  / path-empty

    if (!rawUrl.starts_with("http:") && !rawUrl.starts_with("https:")) // its HttpUrl after all
        return std::nullopt; // ill-formed, scheme is mandatory

    const auto schemeIdx{ rawUrl.find(':') };
    scheme = string_view(rawUrl.data(), schemeIdx);
    rawUrl.remove_prefix(schemeIdx + 1);


    if (const auto hierPartIdx{ rawUrl.find_first_of("?#") }; hierPartIdx == string::npos)
        hierPart = rawUrl; // no query or fragment
    else {
        hierPart = string_view(rawUrl.data(), hierPartIdx);
        char hierPartDelimiter{ rawUrl[hierPartIdx] };

        rawUrl.remove_prefix(hierPartIdx + 1);

        if (hierPartDelimiter == '#')
            fragment = rawUrl; // no query, just fragment
        else {
            auto it{ rg::find(rawUrl, '#') }; // a query and maybe a fragment too

            query = string_view(rawUrl.begin(), it);

            if (it != rawUrl.end())
                fragment = string_view(it + 1, rawUrl.end());
        }
    }

#pragma endregion
#pragma region Scheme

    // scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )

    static auto const checkScheme = [](char c) {
        return isalnum(c) || c == '+' ||  c == '-' || c == '.';
    };

    if (scheme.empty() || !isalnum(scheme[0]) || !rg::all_of(scheme, checkScheme))
        return std::nullopt;

#pragma endregion
#pragma region Auth & UserInfo

    // authority   = [ userinfo "@" ] host [ ":" port ]
    // userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )


    if (hierPart.empty())
        return std::nullopt;

    if (!hierPart.starts_with("//"))
        return std::nullopt; // in HTTP, {"//" authority path-abempty} is mandatory

    hierPart.remove_prefix(2);
    auto pathIdx{ hierPart.find('/') };

    if (pathIdx == 0)
        return std::nullopt; // ill-formed, authority is empty

    if (pathIdx != string::npos) {
        path = string_view(pathIdx + hierPart.begin(), hierPart.end()); // includes '/'

        hierPart.remove_suffix(path.size());
    }


    auto authIdx{ static_cast<long long>(hierPart.find('@')) };

    user = string_view(hierPart.data(), std::max(0LL, authIdx));
    hierPart.remove_prefix(authIdx + 1);

    // TODO: check chars from userinfo

    auto portIdx{ hierPart.find(':') };

    if (portIdx == string_view::npos) // no port
        host = hierPart;
    else {
        host = string_view(hierPart.data(), portIdx);

        // hierPart.rbegin() + 1 is hierPart.end(), but I cant dereference it in MSVC

        auto [ptr, ec] = std::from_chars(&*hierPart.begin() + portIdx + 1, &*hierPart.rbegin() + 1, port);

        hierPart.remove_prefix(portIdx + 1);

        if (ec != std::errc() || ptr != &*hierPart.rbegin() + 1)
            return std::nullopt;
        if (port < 0 || port > 65535)
            return std::nullopt;
    }

    if (host.empty())
        return std::nullopt; // ill-formed, in auth host is mandatory

#pragma endregion
#pragma region Host

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


    // IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet
    // dec-octet   = DIGIT                 ; 0-9
    //             / %x31-39 DIGIT         ; 10-99
    //             / "1" 2DIGIT            ; 100-199
    //             / "2" %x30-34 DIGIT     ; 200-249
    //             / "25" %x30-35          ; 250-255

    // reg-name    = *( unreserved / pct-encoded / sub-delims )


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







    HttpUrl res;

    res.scheme   = scheme;
    res.user     = user;
    res.host     = host;
    res.port     = port;
    res.path     = path;
    res.query    = QueryParams::Parse(query);
    res.fragment = fragment;

    return res;
}

std::string HttpUrl::Encode(std::string_view str) {
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


constexpr static auto hexCharToInt = [] {
    std::array<int, 256> toHex{};
    for (char c{'0'}; c <= '9'; c++) toHex[c] = c - '0';
    for (char c{'a'}; c <= 'z'; c++) toHex[c] = c - 'a' + 10;
    for (char c{'A'}; c <= 'Z'; c++) toHex[c] = c - 'A' + 10;

    return toHex;
}();


std::optional<std::string> HttpUrl::TryDecode(std::string_view str) {
    std::string buffer;
    buffer.reserve(str.length());


    for (int i{}; i < str.size(); i++) {
        if (str[i] == '%') {
            if (i + 2 >= str.length() || !std::isxdigit(str[i + 1]) || !std::isxdigit(str[i + 2]))
                return std::nullopt;

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

bool HttpUrl::operator==(const HttpUrl &) const = default;

HttpUrl::HttpUrl() = default;

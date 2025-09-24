#include <Thoth/Http/QueryParams.hpp>
#include <Thoth/Http/HttpUrl.hpp>
#include <Thoth/Http/HttpHeaders.hpp>
#include "../include/BaseTests.hpp"

using namespace Thoth::Http;

void QueryParamsTests() {
    TestBattery("QueryParams");

    QueryParams params1{
                {"users",      {"KG", "AiKath", "Kai", "LastArchimedes"}},
                {"extensions", {"cxx", "hs", "py", "rs"}},
                {"nums",       {"3", /*.*/ "1", "4", "1", "6"}}
    };
    QueryParams params2{
                {"Can I do",  {"something", "special", "someday?"}},
                {"Can I be",  {"someone", "loveable?"}},
                {"Is there",  {"a meaning", "to", "all of this?"}},

                {"m-cresol:", {"People", "will note", "if I work hard."}},
                {"o-cresol:", {"But Im also ok", "with being", "alone."}},
                {"p-cresol:", {"Maybe my hopes", "were to high", "for my situation."}},

                {"Sorry",     {"about that,", " life sucks."}},
                {"I hate",    {"being human."}},
                {"We hate",   {"loving live."}},
                {"Im Going",  {"back to work now."}}
    };


    Test("Exists 1", params1.Exists("users"));
    Test("Exists 2", params2.Exists("p-cresol:"));
    Test("Exists false 1", !params1.Exists("paradichlorobenzene"));
    Test("Exists false 2", !params2.Exists("antichlorobenzene"));


    auto paramsCpy1{ params1 };
    paramsCpy1["acid"] = {"dont mind"};
    Test("operator[] assign", paramsCpy1.ValExists("acid", "dont mind"));


    paramsCpy1.RemoveKey("acid");
    Test("RemoveKey", !paramsCpy1.Exists("acid"));

    auto paramsCpy2{ params1 };
    Test("Add/Remove init", !paramsCpy2.ValExists("users", "Marcelo"));
    paramsCpy2.Add("users", "Marcelo");
    Test("Add check", paramsCpy2.ValExists("users", "Marcelo"));
    paramsCpy2.Remove("users", "Marcelo");
    Test("Remove check", !paramsCpy2.ValExists("users", "Marcelo"));

    auto paramsCpy3{ params1 };
    Test("SetIfNull existing", !paramsCpy3.SetIfNull("users", "NewValue"));
    Test("SetIfNull new", paramsCpy3.SetIfNull("new_key", "FirstValue"));
    Test("SetIfNull verify", paramsCpy3.ValExists("new_key", "FirstValue"));

    auto usersOpt{ params1.Get("users") };
    Test("Get existing", usersOpt.has_value());
    if (usersOpt)
        Test("Get existing size", usersOpt->get().size() == 4);


    auto nullOpt{ params1.Get("dont hurt") };
    Test("Get non-existent", !nullOpt.has_value());



    auto paramsCpy4{ params2 };
    Test("Initial size", paramsCpy4.Size() == 10);
    paramsCpy4.Clear();
    Test("Clear size", paramsCpy4.Size() == 0);
    Test("Clear check", !paramsCpy4.Exists("Sorry"));



    size_t count{};
    for (const auto& pair : params1) { (void)pair; count++; }
    Test("Iterator count", count == params1.Size());



    std::string_view queryStr{ "users=KG,AiKath,Kai,LastArchimedes&extensions=cxx,hs,py,rs" };
    auto parsedParams = QueryParams::Parse(queryStr);
    Test("Parse size", parsedParams.Size() == 2);
    Test("Parse value", parsedParams.ValExists("users", "Kai"));



    std::string_view encodedStr{ "name=John%20Doe&topic=C%2B%2B%20Programming" };
    auto decodedOpt{ QueryParams::ParseDecodified(encodedStr) };
    Test("ParseDecodified optional", decodedOpt.has_value());
    if(decodedOpt) {
        Test("ParseDecodified space", decodedOpt->ValExists("name", "John Doe"));
        Test("ParseDecodified plus",  decodedOpt->ValExists("topic", "C++ Programming"));
    }



    auto formattedStr{ std::format("{}", params1) };
    auto roundtripParams = QueryParams::Parse(formattedStr);
    Test("Format/Parse Round-trip", std::format("{}", roundtripParams) == formattedStr);
}


void HttpUrlTests() {
    TestBattery("HttpUrl");

    const auto url1Opt{ HttpUrl::FromUrl("https://www.example.com/path/to/resource") };

    Test("Parse Simples opt", url1Opt.has_value());
    if (url1Opt) {
        Test("Simple: scheme parse", url1Opt->scheme == "https");
        Test("Simple: host parse", url1Opt->host == "www.example.com");
        Test("Simple: path parse", url1Opt->path == "/path/to/resource");
        Test("Simple: port parse (default)", url1Opt->port == 0);
        Test("Simple: query parse (empty)", url1Opt->query.Empty());

        Test("Simple: origin", std::format("{:o}", *url1Opt) == "https://www.example.com");
    }



    const auto url2Opt{ HttpUrl::FromUrl("http://user:pass@localhost:8080/api/v1/data?id=123&type=json#details") };

    Test("Complex: opt parse", url2Opt.has_value());
    if (url2Opt) {
        Test("Complex: scheme parse", url2Opt->scheme == "http");
        Test("Complex: user parse", url2Opt->user == "user:pass");
        Test("Complex: host parse", url2Opt->host == "localhost");
        Test("Complex: port parse", url2Opt->port == 8080);
        Test("Complex: path parse", url2Opt->path == "/api/v1/data");
        Test("Complex: query parse", url2Opt->query == QueryParams::Parse("id=123&type=json"));
        Test("Complex: fragment parse", url2Opt->fragment == "details");

        Test("Complex: origin", std::format("{:o}", *url2Opt) == "http://localhost:8080");
    }



    const auto url3Opt{ HttpUrl::FromUrl("https://api.service.com/search?q=c%2B%2B%20programming") };

    Test("Parse Encoding opt", url3Opt.has_value());
    if (url3Opt) {
        Test("Parse encoding host", url3Opt->host == "api.service.com");
        Test("Parse encoding path", url3Opt->path == "/search");
        Test("Parse encoding query", url3Opt->query == QueryParams::Parse("q=c%2B%2B%20programming"));
    }



    Test("Invalid parse: no scheme", !HttpUrl::FromUrl("www.google.com/imghp").has_value());
    Test("Invalid parse: invalid scheme", !HttpUrl::FromUrl("ftp://a.com").has_value());
    Test("Invalid parse: no authority", !HttpUrl::FromUrl("http:/path/to/file").has_value());
    Test("Invalid parse: invalid port", !HttpUrl::FromUrl("http://localhost:xyz/").has_value());
    Test("Invalid parse: to big port", !HttpUrl::FromUrl("https://localhost:65536/").has_value());
    Test("Invalid parse: empty host", !HttpUrl::FromUrl("https://user@/path").has_value());



    constexpr std::string_view originalUrlStr{ "https://user@example.com:8443/a/b?c=d#e" };
    const auto parsedUrlOpt{ HttpUrl::FromUrl(originalUrlStr) };

    Test("Round-trip parse", parsedUrlOpt.has_value());
    if (parsedUrlOpt) {
        const auto parsedUrlOpt2{ HttpUrl::FromUrl(std::format("{}", *parsedUrlOpt)) };
        const auto formattedStr{ std::format("{}", *parsedUrlOpt2) };
        Test("Format/Parse Round-trip", parsedUrlOpt == parsedUrlOpt2);
    }
}

void HttpHeadersTests() {
    TestBattery("HttpHeaders");

    HttpHeaders headers1{
            {"Content-Type", "application/json"},
            {"Authorization", "Bearer token123"},
            {"X-Custom-Header", "custom-value"},
            {"Accept", "text/html,application/xml"}
    };

    HttpHeaders headers2{
            {"Set-Cookie", "session=abc123; Path=/"},
            {"Set-Cookie", "user=john; Domain=.example.com"},
            {"Set-Cookie", "theme=dark; Secure"},
            {"Cache-Control", "no-cache, no-store"},
            {"WWW-Authenticate", "Basic realm=\"Protected\""},
            {"WWW-Authenticate", "Bearer realm=\"API\""}
    };


    Test("Exists case insensitive 1", headers1.Exists("content-type"));
    Test("Exists case insensitive 2", headers1.Exists("AUTHORIZATION"));
    Test("Exists case insensitive 3", headers1.Exists("X-CUSTOM-HEADER"));
    Test("Exists mixed case", headers1.Exists("Accept"));
    Test("Exists false", !headers1.Exists("Non-Existent"));


    Test("Exists pair exact", headers1.Exists("Content-Type", "application/json"));
    Test("Exists pair case insensitive key", headers1.Exists("CONTENT-TYPE", "application/json"));
    Test("Exists pair wrong value", !headers1.Exists("Content-Type", "text/plain"));
    Test("Exists pair non-existent key", !headers1.Exists("Missing-Header", "some-value"));


    auto headersCpy1{ headers1 };
    headersCpy1.Add("X-Rate-Limit", "1000");
    Test("Add new header", headersCpy1.Exists("x-rate-limit"));
    Test("Add verify value", headersCpy1.Exists("X-Rate-Limit", "1000"));


    headersCpy1.Add("Accept", "application/json");
    auto acceptValue{ headersCpy1.Get("accept") };
    Test("Add comma separation", acceptValue && acceptValue->get().find("text/html,application/xml, application/json") != std::string::npos);


    auto headersCpy2{ headers2 };
    headersCpy2.Add("Set-Cookie", "temp=xyz; Max-Age=3600");
    auto setCookies{ headersCpy2.GetSetCookie() };
    Test("Add Set-Cookie count", setCookies.size() == 4);
    Test("Add Set-Cookie contains new", std::ranges::find(setCookies, "temp=xyz; Max-Age=3600") != setCookies.end());


    auto headersCpy3{ headers1 };
    headersCpy3.Set("Content-Type", "text/plain");
    Test("Set replace existing", headersCpy3.Exists("Content-Type", "text/plain"));
    Test("Set replace verify old gone", !headersCpy3.Exists("Content-Type", "application/json"));

    headersCpy3.Set("New-Header", "new-value");
    Test("Set add new", headersCpy3.Exists("new-header", "new-value"));


    auto headersCpy4{ headers2 };
    headersCpy4.Set("Set-Cookie", "replaced=value");
    auto setCookiesAfterSet{ headersCpy4.GetSetCookie() };
    Test("Set Set-Cookie adds not replaces", setCookiesAfterSet.size() == 4);


    auto headersCpy5{ headers1 };
    Test("Remove existing", headersCpy5.Remove("Content-Type", "application/json"));
    Test("Remove verify gone", !headersCpy5.Exists("Content-Type"));
    Test("Remove non-existent pair", !headersCpy5.Remove("Content-Type", "wrong-value"));
    Test("Remove non-existent header", !headersCpy5.Remove("Missing-Header", "any-value"));


    auto headersCpy6{ headers1 };
    Test("Remove case insensitive", headersCpy6.Remove("AUTHORIZATION", "Bearer token123"));
    Test("Remove case insensitive verify", !headersCpy6.Exists("authorization"));


    auto headersCpy7{ headers1 };
    Test("SetIfNull existing key", !headersCpy7.SetIfNull("Content-Type", "new-value"));
    Test("SetIfNull existing unchanged", headersCpy7.Exists("Content-Type", "application/json"));
    Test("SetIfNull new key", headersCpy7.SetIfNull("Fresh-Header", "fresh-value"));
    Test("SetIfNull new verify", headersCpy7.Exists("fresh-header", "fresh-value"));


    Test("SetIfNull case insensitive existing", !headersCpy7.SetIfNull("CONTENT-TYPE", "another-value"));


    auto contentTypeOpt{ headers1.Get("Content-Type") };
    Test("Get existing", contentTypeOpt.has_value());
    if (contentTypeOpt)
        Test("Get value correct", contentTypeOpt->get() == "application/json");

    auto missingOpt{ headers1.Get("Missing-Header") };
    Test("Get non-existent", !missingOpt.has_value());


    auto authOpt{ headers1.Get("AUTHORIZATION") };
    Test("Get case insensitive", authOpt.has_value());
    if (authOpt)
        Test("Get case insensitive value", authOpt->get() == "Bearer token123");


    const HttpHeaders& constHeaders{ headers1 };
    auto constGetOpt{ constHeaders.Get("accept") };
    Test("Const Get", constGetOpt.has_value());


    auto setCookieValues{ headers2.GetSetCookie() };
    Test("GetSetCookie count", setCookieValues.size() == 3);
    Test("GetSetCookie contains session", std::ranges::find(setCookieValues, "session=abc123; Path=/") != setCookieValues.end());
    Test("GetSetCookie contains user", std::ranges::find(setCookieValues, "user=john; Domain=.example.com") != setCookieValues.end());
    Test("GetSetCookie contains theme", std::ranges::find(setCookieValues, "theme=dark; Secure") != setCookieValues.end());


    HttpHeaders headersNoSetCookie{ {"Content-Type", "text/html"} };
    auto emptySetCookies{ headersNoSetCookie.GetSetCookie() };
    Test("GetSetCookie empty", emptySetCookies.empty());


    auto setCookieView{ headers2.GetSetCookieView() };
    std::vector<std::string> viewValues;
    for (const auto& cookie : setCookieView)
        viewValues.emplace_back(cookie);

    Test("GetSetCookieView count", viewValues.size() == 3);


    auto headersCpy8{ headers1 };
    Test("operator[] existing", headersCpy8["content-type"] == "application/json");
    Test("operator[] case insensitive", headersCpy8["ACCEPT"] == "text/html,application/xml");


    auto& newValue{ headersCpy8["Brand-New-Header"] };
    newValue = "assigned-value";
    Test("operator[] create new", headersCpy8.Exists("brand-new-header"));
    Test("operator[] create value", headersCpy8.Exists("brand-new-header", "assigned-value"));


    Test("Size check", headers1.Size() == 4);
    Test("Empty false", !headers1.Empty());

    HttpHeaders emptyHeaders{};
    Test("Empty true", emptyHeaders.Empty());
    Test("Empty size zero", emptyHeaders.Size() == 0);

    auto headersCpy9{ headers1 };
    headersCpy9.Clear();
    Test("Clear size", headersCpy9.Size() == 0);
    Test("Clear empty", headersCpy9.Empty());
    Test("Clear no exists", !headersCpy9.Exists("Content-Type"));


    size_t iterCount{};
    for (const auto& pair : headers1) {
        (void)pair;
        iterCount++;
    }
    Test("Iterator count", iterCount == headers1.Size());


    bool allLowercase{ true };
    for (const auto &key: headers1 | std::views::keys) {
        for (char c : key) {
            if ('A' <= c && c <= 'Z') {
                allLowercase = false;
                break;
            }
        }
        if (!allLowercase) break;
    }
    Test("Iterator keys lowercase", allLowercase);


    HttpHeaders headers1Copy{ headers1 };
    Test("Equality same", headers1 == headers1Copy);

    headers1Copy.Add("Extra-Header", "extra-value");
    Test("Equality different", headers1 != headers1Copy);


    HttpHeaders::MapType vectorInit{
            {"Host", "example.com"},
            {"User-Agent", "TestClient/1.0"}
    };
    HttpHeaders headersFromVector{ vectorInit };
    Test("Vector constructor size", headersFromVector.Size() == 2);
    Test("Vector constructor keys lowercase", headersFromVector.Exists("host"));
    Test("Vector constructor values", headersFromVector.Exists("user-agent", "TestClient/1.0"));


    HttpHeaders headersWithEmpty{ {"Empty-Header", ""} };
    Test("Empty value exists", headersWithEmpty.Exists("empty-header"));
    Test("Empty value get", headersWithEmpty.Get("empty-header")->get().empty());


    HttpHeaders headersWithSpaces{ {"Spaced-Header", "  value with spaces  "} };
    Test("Spaced value exact", headersWithSpaces.Exists("spaced-header", "  value with spaces  "));


    HttpHeaders multipleHeaders{};
    multipleHeaders.Add("Accept-Encoding", "gzip");
    multipleHeaders.Add("Accept-Encoding", "deflate");
    multipleHeaders.Add("Accept-Encoding", "br");
    auto encodingOpt{ multipleHeaders.Get("accept-encoding") };
    Test("Multiple same key", encodingOpt && encodingOpt->get() == "gzip, deflate, br");



    HttpHeaders simpleHeaders{
            {"content-type", "text/html"},
            {"server", "nginx/1.18"}
    };
    auto simpleFormatted{ std::format("{}", simpleHeaders) };
    Test("Format exact simple", simpleFormatted == "content-type: text/html\r\nserver: nginx/1.18\r\n");

    HttpHeaders repeatHeaders{
            {"accept-encoding", "gzip"},
            {"accept-encoding", "deflate"}
    };
    auto repeatFormatted{ std::format("{}", repeatHeaders) };
    Test("Format exact repeat", repeatFormatted == "accept-encoding: gzip\r\naccept-encoding: deflate\r\n");


    HttpHeaders setCookieHeaders{
            {"set-cookie", "session=abc123"},
            {"set-cookie", "user=john"}
    };
    auto setCookieExactFormatted{ std::format("{}", setCookieHeaders) };
    Test("Format exact set-cookie", setCookieExactFormatted == "set-cookie: session=abc123\r\nset-cookie: user=john\r\n");
}

void HttpRequestTests() {

}
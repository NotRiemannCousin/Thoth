#include <Thoth/Http/QueryParams.hpp>
#include <Thoth/Http/HttpUrl.hpp>
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
    Test("Exists false 1", !params1.Exists("parachlorobenzene"));
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


void HttpRequestTests(){

}



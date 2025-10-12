#include <print>
#include <Thoth/Http/Request/HttpRequest.hpp>
#include <Thoth/Http/HttpMethods/GetHttpMethod.hpp>

#include <Thoth/Http/HttpClient.hpp>


namespace Http = Thoth::Http;

int main() {
    Thoth::Http::HttpRequest<Http::HttpGetMethod> request{
        // *Http::HttpUrl::FromUrl({ "https://api.discogs.com/artists/4001234" }),
        *Http::HttpUrl::FromUrl({ "http://neverssl.com" }),
        {},
        Http::HttpVersion::HTTP1_1,
        Http::HttpHeaders{
            { "Accept", "*/*" },
            { "User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/140.0.0.0 Safari/537.36 Edg/140.0.0.0" },
            { "Keep-Alive", "true" },
            { "Accept-Encoding", "identity" }
        },
    };

    const auto response{ Http::HttpClient::Send(request) };

    if (response) {
        std::print("status: {} {}\n"
            "headers:\n"
            "{}\n"
            "body:\n"
            "{}",
            static_cast<int>(response->status), response->statusMessage, response->headers, response->body);
    }else {
        std::print("{}", response.error());

        const int error{ WSAGetLastError() };

        if (error != 0)
            std::print("\n{}", error);
    }


    return 0;
}

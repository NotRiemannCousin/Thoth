#include <print>
#include <Thoth/Http/HttpClient.hpp>


namespace NHttp = Thoth::Http;

int main() {
    const NHttp::HttpRequest<NHttp::HttpGetMethod> request{
        *NHttp::HttpUrl::FromUrl({ "https://api.chucknorris.io/jokes/random" })
        // It's a good practice to validate the Url outside
    };

    const auto response{ NHttp::HttpClient::Send(request) };

    if (response) {
        std::print("status: {} {}\n"
            "headers:\n"
            "{}\n"
            "body:\n"
            "{}",
            static_cast<int>(response->status), response->statusMessage, response->headers, response->body);
    } else {
        std::print("{}", response.error());

        const int error{ WSAGetLastError() }; // ... Just in case

        if (error != 0)
            std::print("\n{}", error);
    }

    return 0;
}

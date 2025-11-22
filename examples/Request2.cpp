#include <print>
#include <Thoth/Http/Client.hpp>


namespace NHttp = Thoth::Http;

int main() {
    const auto response{
        NHttp::GetRequest::FromUrl({ "https://api.chucknorris.io/jokes/random" })
                .transform(NHttp::Client::Send<>)
                .value_or(std::unexpected{ "Failed to connect." })
    };

    if (response) {
        std::print("status: {} {}\n"
            "headers:\n"
            "{}\n"
            "body:\n"
            "{}",
            static_cast<int>(response->status), response->statusMessage, response->headers, response->body);
    } else {
        std::println("Error: {}", response.error());

        const int error{ WSAGetLastError() };

        if (error != 0)
            std::print("\n{}", error);
    }

    return 0;
}

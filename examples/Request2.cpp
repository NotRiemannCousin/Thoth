#include <print>
#include <Thoth/Http/Client.hpp>


namespace NHttp = Thoth::Http;

int main() {
    const auto response{
        NHttp::GetRequest::FromUrl({ "https://api.chucknorris.io/jokes/random" })
                .and_then(NHttp::Client::Send<>)
    };

    if (response) {
        std::print("status: {} {}\n"
            "headers:\n"
            "{}\n"
            "body:\n"
            "{}",
            static_cast<int>(response->status), response->statusMessage, response->headers, response->body);
    } else {
        std::println("{}", response.error());

        const int error{ WSAGetLastError() };

        if (error != 0)
            std::print("\nWSA Error: {}", error);
    }

    return 0;
}

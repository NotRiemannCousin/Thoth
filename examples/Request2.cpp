#include <print>
#include <Thoth/Http/Client/Client.hpp>


namespace NHttp = Thoth::Http;

void PrintResponse(const Thoth::Http::GetResponse& response) {
    std::print(
        "status: {} {}\n"
        "headers:\n"
        "{}\n"
        "body:\n"
        "{}",
        static_cast<int>(response.status), response.statusMessage, response.headers, response.body
    );
}

void PrintError(Thoth::ThothError err) {
    std::println("{}", err);

    if (const auto error{ Hermes::GetError() }; !error)
        std::print("\nWSA Error: {}", error.error());
}


int main() {
    const auto response{
        NHttp::GetRequest::FromUrl({ "https://api.chucknorris.io/jokes/random" })
                .and_then(NHttp::Client::H_Send())
    };

    if (response) PrintResponse(*response);
    else          PrintError(response.error());

    return 0;
}

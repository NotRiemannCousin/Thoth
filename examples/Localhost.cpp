#include <Thoth/Http/Client.hpp>

namespace NHttp = Thoth::Http;
namespace NJson = Thoth::NJson;
using NJson::Json;

int main() {
    auto body{ Thoth::Http::GetRequest::FromUrl("https://localhost:4433/")
            .and_then(Thoth::Http::Client::H_Send())
            .transform(&Thoth::Http::GetResponse::MoveBody) };

    if (body)
        std::print("{}", *body);
    else {

        std::print("{}\n\n{}", body.error(), WSAGetLastError());
    }
}
#include <Thoth/Http/Client/Client.hpp>

namespace NHttp = Thoth::Http;
namespace NJson = Thoth::NJson;
using NJson::Json;

int main() {
    auto body{ NHttp::GetRequest::FromUrl("https://localhost:4433/")
            .and_then(NHttp::Client::H_Send())
            .transform(&NHttp::GetResponse::MoveBody) };

    if (body) std::print("{}", *body);
    else      std::print("{}\n\n{}", body.error(), WSAGetLastError());
}
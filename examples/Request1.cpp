#pragma warning(disable: 4455)
#include <print>

#include <Thoth/Http/Client.hpp>
#include <Thoth/Utils/Functional.hpp>


namespace NHttp = Thoth::Http;
namespace NJson = Thoth::NJson;
using NJson::Json;

std::expected<std::monostate, std::string> FunctionalRequest() {
    using std::string_literals::operator ""s;
    namespace Utils = Thoth::Utils;

    static constexpr auto s_printSingleMember = [](const NJson::Object* obj) {
        std::print("{}\n", (**obj)["name"]);
        return std::monostate{};
    };

    static constexpr auto s_printOrDie = [](const NJson::Array& members) -> std::expected<std::monostate, std::string> {
        for (const auto& member : members)
            member.EnsureRef<NJson::Object>()
                .transform(s_printSingleMember);

        return std::monostate{};
    };

    return NHttp::GetRequest::FromUrl("https://api.discogs.com/artists/4001234")
            .transform(NHttp::Client::Send<>)
            .value_or(std::unexpected{ "Failed to connect." })
            .transform(&NHttp::GetResponse::AsJson)
            .and_then(Utils::ValueOrHof<Json>("Cant convert to json."s))

            .transform(std::bind_back(&Json::GetAndMove, "members" ))

            .transform(&Json::EnsureMov<NJson::Array>)
            .and_then(Utils::ValueOrHof<NJson::Array>("'members' array doesn't exist."s))

            .and_then(s_printOrDie);
}

std::expected<std::monostate, std::string> OtherExample() {
    using namespace Thoth::Utils;
    using namespace Thoth::Http;
    using std::string_literals::operator ""s;

    return GetRequest::FromUrl("https://api.jikan.moe/v4/anime/57555")
            .transform(Client::Send<>)
            .value_or(std::unexpected{ "Failed to connect." })
            .transform(&GetResponse::AsJson)
            .and_then(ValueOrHof<Json>("Cant convert to json."s))

            .transform([](const Json& json){ return std::print("{}", json), std::monostate{}; });
}


int main() {
    const auto res{ FunctionalRequest() };

    if (!res) {
        std::print("{}", res.error());

        if (const int error{ WSAGetLastError() }; error != 0)
            std::print("\nWSA error: {}", error);
    }

    return 0;
}

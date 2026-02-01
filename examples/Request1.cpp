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

    const auto membersOrError {
        NHttp::GetRequest::FromUrl("https://api.discogs.com/artists/4001234")
                .transform(NHttp::Client::Send<>)
                .value_or(std::unexpected{ "Failed to connect." })
                .transform(&NHttp::GetResponse::AsJson)
#ifdef DENSE_DEBUG_JSON
                // Enables DENSE_DEBUG_JSON at cmake to be able to see.
                .transform([](auto&& val) {
                    std::print("\033[2J\033[H");
                    std::print("\nCalls after the parsing:\n\n");
                    return val;
                })
#endif
                .and_then(Utils::ValueOrHof<Json>("Cant convert to json."s))
                .transform(std::bind_back(&Json::GetAndMove, "members" ))
                // or `.transform(std::bind_back<Json::Json::GetAndMove>("members"))` with C++26.
                // GetAndMove is &&-qualified: only rvalues can call it, so no copies are made.
                // In chained calls the value is consumed at each step, so it's always a last use,
                // making moves safe. If you need to avoid moving, use Get() or GetCopy() on a const&.
                .and_then(Utils::ValueOrHof<NJson::ValWrapper>(R"("members" doesn't exist.)"s))
                .and_then(Utils::ErrorIfNotHof<&Json::IsOf<NJson::Array>>(R"("members" isn't an array.)"s))
    };

    // Without comments:
    // const auto membersOrError {
    //     NHttp::GetRequest::FromUrl("https://api.discogs.com/artists/4001234")
    //             .transform(NHttp::Client::Send<>)
    //             .value_or(std::unexpected{ "Failed to connect." })
    //             .transform(&NHttp::GetResponse::AsJson)
    //             .and_then(Utils::ValueOrHof<Json>("Cant convert to json."s))
    //             .transform(std::bind_back(&Json::GetAndMove, "members" ))
    //             .and_then(Utils::ValueOrHof<NJson::ValWrapper>(R"("members" doesn't exist.)"s))
    //             .and_then(Utils::ErrorIfNotHof<&Json::IsOf<NJson::Array>, Json>(R"("members" isn't an array.)"s))
    // };

    if (!membersOrError)
        return std::unexpected{ membersOrError.error() };

    for (const auto& member : membersOrError->As<NJson::Array>()) {
        if (!member.IsOf<NJson::Object>())
            return std::unexpected{ R"(Value isn't an object)" };

        std::print("{}\n", (*member.As<NJson::Object>())["name"]);
    }

    return {};
}


// Or if you are confident enough...
// Advice: no verifications are made so use it inside a try-catch block.
std::expected<std::monostate, std::string> ShortRequest() {
    using namespace Thoth::Http;
    using namespace Thoth::Utils;

    using std::string_literals::operator ""s;

    const auto members{
        GetRequest::FromUrl("https://api.discogs.com/artists/4001234")
        .transform(Client::Send<>)
        .transform(&GetResponse::AsJson)
        .transform(std::bind_back(&Json::GetAndMove, "members"))
        .value().value()
    };

    for (const auto& member : members.As<NJson::Array>())
        std::print("{}\n", (*member.As<NJson::Object>())["name"]);

    return {};
}

std::expected<std::monostate, std::string> OtherExample() {
    using namespace Thoth::Utils;
    using namespace Thoth::Http;
    using std::string_literals::operator ""s;

    const auto json{
        GetRequest::FromUrl("https://api.jikan.moe/v4/anime/57555")
                .transform(Client::Send<>)
                .value_or(std::unexpected{ "Failed to connect." })
                .transform(&GetResponse::AsJson)
                .and_then(ValueOrHof<Json>("Cant convert to json."s))
    };

    if (!json)
        return std::unexpected{ json.error() };

    return {};
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

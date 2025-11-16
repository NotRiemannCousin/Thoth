#pragma warning(disable: 4455)
#include <print>

#include <Thoth/Http/HttpClient.hpp>
#include <Thoth/Utils/Functional.hpp>


namespace Http = Thoth::Http;
namespace Json = Thoth::Json;

std::expected<std::monostate, std::string> Request() {
    const auto request{ Http::GetRequest::FromUrl("https://api.discogs.com/artists/4001234") };

    if (!request)
        return std::unexpected{ "Can't connect to FromUrl" };

    const auto response{ Http::HttpClient::Send(*request) };

    if (!response)
        return std::unexpected{ response.error() };

    const auto json{ response->AsJson() };

    if (!json)
        return std::unexpected{ "Cannot parse json" };


    const auto membersOpt{ json->Get("members") };
    if (!membersOpt)
        return std::unexpected{ R"("members" doesn't exist)" };

    const auto& members{ membersOpt->get() };

    if (!members.IsOf<Json::Array>())
        return std::unexpected{ R"("members" isn't an array)" };

    for (const auto& member : members.As<Json::Array>()) {
        if (!member.IsOf<Json::Object>())
            return std::unexpected{ R"("members"'s child isn't an object)" };

        std::print("{}\n", (*member.As<Json::Object>())["name"]);
        // or std::print("{}\n", (*member.As<Json::Object>())["name"].As<Json::String>().AsRef());
    }

    return {};
}

std::expected<std::monostate, std::string> FunctionalRequest() {
    namespace Utils = Thoth::Utils;


    using std::string_literals::operator ""s;


    using Utils::ConstFn;

    const auto membersOrError {
        Http::GetRequest::FromUrl("https://api.discogs.com/artists/4001234")
                .transform(Http::HttpClient::Send<>)
                .value_or(std::unexpected{ "Failed to connect." })
                .transform(&Http::GetResponse::AsJson)
#ifdef DENSE_DEBUG_JSON
                // Enables DENSE_DEBUG_JSON at cmake to be able to see.
                .transform([](auto&& val) {
                    std::print("\033[2J\033[H");
                    std::print("Just calls after the parsing:\n\n\n");
                    return val;
                })
#endif
                .and_then(Utils::ValueOrHof<std::optional<Json::Json>&&>("Cant convert to json."s))
                .transform(std::bind_back(&Json::Json::GetMove, "members"))
                // or `.transform(std::bind_back<Json::Json::GetMove>("members"))` with C++26.
                // GetMove is &&-qualified: only rvalues can call it, so no copies are made.
                // In chained calls the value is consumed at each step, so it's always a last use,
                // making moves safe. If you need to avoid moving, use Get() or GetCopy() on a const&.
                .and_then(Utils::ValueOrHof<Json::Json::OptValWrapper&&>(R"("members" doesn't exist.)"s))
    };

    if (!membersOrError)
        return std::unexpected{ membersOrError.error() };

    const auto& members{ membersOrError.value() };

    if (!members.IsOf<Json::Array>())
        return std::unexpected{ R"("members" isn't an array)" };

    for (const auto& member : members.As<Json::Array>()) {
        if (!member.IsOf<Json::Object>())
            return std::unexpected{ R"(Value isn't an object)" };

        std::print("{}\n", (*member.As<Json::Object>())["name"]);
    }

    return {};
}


// Or if you are confident enough...
// Advice: no verifications are made so use it inside a try-catch block.
std::expected<std::monostate, std::string> ShortRequest() {
    using namespace Thoth::Http;
    using namespace Thoth::Utils;

    using std::string_literals::operator ""s;

    const auto request{GetRequest::FromUrl("https://api.discogs.com/artists/4001234") };

    const auto response{ *HttpClient::Send(*request) };
    const auto json{ *response.AsJson() };

    const auto& members{ json.Get("members")->get() };

    for (const auto& member : members.As<Json::Array>())
        std::print("{}\n", (*member.As<Json::Object>())["name"]);

    return {};
}

std::expected<std::monostate, std::string> FunctionalShortRequest() {
    using namespace Thoth::Utils;
    using namespace Thoth::Http;

    const auto members{
        GetRequest::FromUrl("https://api.discogs.com/artists/4001234")
                .transform(HttpClient::Send<>)
                .transform(&GetResponse::AsJson)
                .transform(std::bind_back(MutFn(&Json::Json::Get), "members"))
                .value()->get()
    };

    for (const auto& member : members.As<Json::Array>())
        std::print("{}\n", (*member.As<Json::Object>())["name"]);

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

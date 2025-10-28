#include <print>
#include <Thoth/Json/Json.hpp>

#include <Thoth/Http/HttpClient.hpp>


namespace Http = Thoth::Http;
namespace Json = Thoth::Json;

std::expected<std::monostate, std::string> MakeRequest() {
    const Http::HttpRequest<Http::HttpGetMethod> request{
        *Http::HttpUrl::FromUrl({ "https://api.discogs.com/artists/4001234" })
    };

    const auto response{ Http::HttpClient::Send(request) };

    if (!response)
        return std::unexpected{ response.error() };

    const auto json{ response->AsJson() };

    if (!json)
        return std::unexpected{ "Cannot parse json" };


    const auto membersOpt{ json->Get("members") };
    if (!membersOpt)
        return std::unexpected{ "\"members\" doesn't exists" };

    const auto& members{ membersOpt->get() };

    if (!members.IsOf<Json::Array>())
        return std::unexpected{ "\"members\" isn't an array" };

    for (const auto& member : members.As<Json::Array>()) {
        if (!member.IsOf<Json::Object>())
            return std::unexpected{ "\"members\"'s child isn't an object" };


        std::print("{}\n", (*member.As<Json::Object>())["name"]);
    }

    return {};
}

// Or if you are confident enough..
// I's a good choice to call this function inside an try block
std::expected<std::monostate, std::string> MakeRequestShortVersion() {
    const Http::HttpRequest<Http::HttpGetMethod> request{
        *Http::HttpUrl::FromUrl({ "https://api.discogs.com/artists/4001234" })
    };

    const auto response{ *Http::HttpClient::Send(request) };
    const auto json{ *response.AsJson() };

    const auto& members{ json.Get("members")->get() };

    for (const auto& member : members.As<Json::Array>())
        std::print("{}\n", (*member.As<Json::Object>())["name"]);

    return {};
}


int main() {

    const auto res{ MakeRequest() };

    if (!res) {
        std::print("{}", res.error());

        if (const int error{ WSAGetLastError() }; error != 0)
            std::print("\nWSA error: {}", error);
    }

    return 0;
}

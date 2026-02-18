#pragma warning(disable: 4455)
#include <print>

#include <Thoth/Http/Client.hpp>
#include <Thoth/Utils/Functional.hpp>


namespace NHttp = Thoth::Http;
namespace NJson = Thoth::NJson;
using NJson::Json;

std::expected<std::vector<Json>, NHttp::RequestError> GetMembers(size_t id) {
    using std::string_literals::operator ""s;
    namespace Utils = Thoth::Utils;

    //trying to make the request, send to the server and then convert the body to JSON.
    return NHttp::GetRequest::FromUrl(std::format("https://api.discogs.com/artists/{}", id))
            // All these functions have the same error type `RequestError`, a std::variant with each specific error.
            .and_then(NHttp::Client::Send<>)
            .and_then(&NHttp::GetResponse::AsJson)

            // selecting "members" in the first object
            .and_then(std::bind_back(&Json::GetAndMoveOrError, "members" ))

            // making sure that it's an array
            .and_then(&Json::EnsureMovOrError<NJson::Array>);
}



int main() {
    static constexpr auto s_getName = [](const Json& member) {
        return member.Get("name")
                .and_then(&Json::EnsureRef<NJson::String>)
                .transform(&NJson::String::AsCopy) // converting from Thoth Strings (that are COW) to std::string
                .value_or("<unnamed>");
    };

    static auto constexpr s_printNames = [](auto&& names) {
        std::println("- Members:");
        for (string&& name : names)
            std::println("{}", name);

        return std::monostate{};
    };

    static auto constexpr s_errorHandler = [](auto&& error) {
        std::println("An error occurred: {}", error);

        if (const int wsaError{ WSAGetLastError() }; wsaError != 0)
            std::println("WSA error: {}", wsaError);

        return std::monostate{};
    };


    GetMembers(4001234)
            .transform(std::views::transform(s_getName))
            .transform(s_printNames)
            .transform_error(s_errorHandler);

    return 0;
}

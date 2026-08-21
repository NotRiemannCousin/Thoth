#pragma warning(disable: 4455)
#include <print>

#include <Thoth/Http/Client/Client.hpp>


namespace NHttp = Thoth::Http;
namespace NJson = Thoth::NJson;
using NJson::Json;

std::expected<std::vector<Json>, Thoth::ThothError> GetMembers(size_t id) {
    using std::string_literals::operator ""s;
    namespace Utils = Thoth::Utils;

    //trying to make the request, send to the server and then convert the body to JSON.
    return NHttp::GetRequest::FromUrl(std::format("https://api.discogs.com/artists/{}", id))
            // All these functions have the same error type `ThothError`, a std::variant with each specific error.
            .and_then(NHttp::Client::H_Send())
            .and_then(&NHttp::Response<>::AsJson<>)

            // selecting "members" in the first object
            .and_then(std::bind_back(&Json::GetAndMoveOrError, "members" ))

            // making sure that it's an array
            .and_then(&Json::EnsureMovOrError<NJson::Array>);
}



int main() {
    static constexpr auto getName = [](const Json& member) {
        return member.Get("name")
                .and_then(&Json::EnsureRef<NJson::String>)
                .transform(&NJson::String::AsCopy) // converting from Thoth Strings (that are COW) to std::string
                .value_or("<unnamed>");
    };

    static constexpr auto printNames = [](auto&& names) {
        std::println("- Members:");
        for (std::string&& name : names)
            std::println("{}", name);

        return std::monostate{};
    };

    static constexpr auto errorHandler = [](auto&& error) {
        std::println("An error occurred: {}", error);

        if (const int wsaError{ WSAGetLastError() }; wsaError != 0)
            std::println("WSA error: {}", wsaError);

        return std::monostate{};
    };


    auto _{
        GetMembers(4001234)
                .transform(std::views::transform(getName))
                .transform(printNames)
                .transform_error(errorHandler)
    };

    return 0;
}

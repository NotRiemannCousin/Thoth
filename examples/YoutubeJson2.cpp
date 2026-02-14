#include <print>
#include <chrono>

#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/Client.hpp>
#include <Thoth/Utils/Functional.hpp>


std::expected<std::monostate, string> MakeRequest() {
    namespace NHttp = Thoth::Http;
    namespace Utils = Thoth::Utils;
    namespace NJson = Thoth::NJson;

    using Json = NJson::Json;

    static const std::array<NJson::Key, 8> contentKeys{
        "contents",
        "twoColumnBrowseResultsRenderer",
        "tabs",
        3,
        "tabRenderer",
        "content",
        "richGridRenderer",
        "contents",
    };

    static const array<NJson::Key, 4> albumNameKey{ "richItemRenderer","content", "playlistRenderer", "title" };




    constexpr auto s_extractJson = [](const string& body) {
        constexpr string_view target{ "ytInitialData = " };
        const auto jsonStart{ body.substr(body.find(target) + target.size()) };

        return Json::ParseText(jsonStart, true, false);
    };

    constexpr auto s_printAlbums = [](NJson::Array&& albums) -> std::monostate{

        for (const auto& album: albums) {
            const auto name{ album.Find(albumNameKey) };
            if (name)
                std::println("{}", **name);
        }

        return std::monostate{};
    };

    using std::operator ""s;

    return NHttp::GetRequest::FromUrl("https://www.youtube.com/@ringosheenaofficial/releases")
                .and_then(NHttp::Client::Send<>)

                .transform(&NHttp::Response<>::MoveBody)
                .and_then(s_extractJson)

                .transform(std::bind_back(&Json::FindAndMove, contentKeys))
                .and_then(Utils::ValueOrHof<Json>("Can't find content."s))

                .transform(&Json::EnsureMov<NJson::Array>)
                .and_then(Utils::ValueOrHof<NJson::Array>("Structure isn't an array."s))

                .transform(s_printAlbums);
}


int main() {

    if (const auto err{ MakeRequest() }; !err)
        std::print("{}", err.error());


    return 0;
}
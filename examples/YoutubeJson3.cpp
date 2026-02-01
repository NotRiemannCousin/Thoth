#include <print>
#include <chrono>
#pragma warning(disable: 4455)

#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/Client.hpp>
#include <Thoth/Utils/Functional.hpp>

std::expected<std::monostate, string> PrintInfo(string_view id) {
#pragma region Aliases and Key definitions
    namespace NHttp = Thoth::Http;
    namespace Utils = Thoth::Utils;

    using Thoth::NJson::Key;
    using CRef = Thoth::NJson::CRefValWrapper;

    using Thoth::NJson::Json;
    using Thoth::NJson::Array;
    using Thoth::NJson::String;


    using std::operator ""s;

    std::array<Key, 8> musicTabKeys{ "contents", "singleColumnBrowseResultsRenderer", "tabs",
                0, "tabRenderer", "content", "sectionListRenderer", "contents" };
    std::array<Key, 2> tabContentKeys{ "musicCarouselShelfRenderer", "contents" };
    std::array<Key, 7> tabTitleKeys{ "musicCarouselShelfRenderer", "header",
                "musicCarouselShelfBasicHeaderRenderer", "title", "runs", 0, "text" };
    std::array<Key, 5> albumNameKeys{ "musicTwoRowItemRenderer", "title", "runs", 0, "text" };
    std::array<Key, 4> moreContentButtonKeys{ "musicCarouselShelfRenderer", "header",
                "musicCarouselShelfBasicHeaderRenderer", "moreContentButton" };
#pragma endregion

    std::string body{ R"(
{
    "videoId": "wJ0xnyX70Y4",
    "context": {
        "client": {
             "clientName": "ANDROID",
             "clientVersion": "20.51"
        }
    }
}
    )", };

    const auto contentTabs{ // keeping the json alive
        NHttp::PostRequest::FromUrl("https://music.youtube.com/youtubei/v1/player?prettyPrint=false", body)
                .transform(NHttp::Client::Send<Thoth::Http::PostMethod>)
                .value_or(std::unexpected{ "Failed to connect." })

                .transform(&NHttp::PostResponse::AsJson)
                .and_then(Utils::ValueOrHof<Json>("Can't convert to json."s))
    };
    if (!contentTabs)
        return std::unexpected{ contentTabs.error() };

    std::print("{}", *contentTabs);

    return {};
}


int main() {

    /* "UCTmoyDN-uokTbzk_xXKcx6w" */
    if (const auto oper{ PrintInfo("UCTmoyDN-uokTbzk_xXKcx6w") }; !oper)
        std::println("{}", oper.error());

    return 0;
}
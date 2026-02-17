#include <print>
#include <chrono>
#pragma warning(disable: 4455)

#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/Client.hpp>
#include <Thoth/Utils/Functional.hpp>

std::expected<std::monostate, RequestError> PrintInfo(string_view id) {
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

    using Obj = Thoth::NJson::JsonObject;

    const Obj body{
        { "videoId", "wJ0xnyX70Y4" },
        { "context", Obj{
            { "client", Obj{
                { "clientName", "ANDROID" },
                { "clientVersion", "20.51" }
            } }
        } }
    };

    return
        NHttp::PostRequest::FromUrl("https://music.youtube.com/youtubei/v1/player?prettyPrint=false", body)
                .and_then(NHttp::Client::Send<Thoth::Http::PostMethod>)
                .and_then(&NHttp::PostResponse::AsJson)

                .transform([](const Json& content){ return std::print("{}", content), std::monostate{}; });
}


int main() {

    /* "UCTmoyDN-uokTbzk_xXKcx6w" */
    if (const auto oper{ PrintInfo("UCTmoyDN-uokTbzk_xXKcx6w") }; !oper)
        std::println("{}", oper.error());

    return 0;
}
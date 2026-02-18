#include <print>
#include <chrono>
#pragma warning(disable: 4455)

#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/Client.hpp>
#include <Thoth/Utils/Functional.hpp>

std::expected<std::monostate, Thoth::Http::RequestError> PrintInfo(string_view id) {
#pragma region Aliases and Key definitions
    namespace NHttp = Thoth::Http;
    namespace Utils = Thoth::Utils;

    using Thoth::NJson::Key;
    using CRef = Thoth::NJson::CRefValWrapper;

    using Thoth::NJson::Json;
    using Thoth::NJson::Array;
    using Thoth::NJson::JsonObject;
    using Thoth::NJson::String;


    using std::operator ""s;

    static std::array<Key, 8> musicTabKeys{ "contents", "singleColumnBrowseResultsRenderer", "tabs",
                0, "tabRenderer", "content", "sectionListRenderer", "contents" };
    static std::array<Key, 7> tabTitleKeys{ "musicCarouselShelfRenderer", "header",
                "musicCarouselShelfBasicHeaderRenderer", "title", "runs", 0, "text" };
    static std::array<Key, 5> albumNameKeys{ "musicTwoRowItemRenderer", "title", "runs", 0, "text" };
    static std::array<Key, 2> tabContentKeys{ "musicCarouselShelfRenderer", "contents" };
    static std::array<Key, 4> moreContentButtonKeys{ "musicCarouselShelfRenderer", "header",
                "musicCarouselShelfBasicHeaderRenderer", "moreContentButton" };
#pragma endregion



    static constexpr auto s_getTab = [](const string& name) {
        return [&](const Json& tab) {
            if (const auto title{ tab.Find(tabTitleKeys) }; title)
                return **title == Json{ name };
            return false;
        };
    };

    static constexpr auto s_printAlbumName = [](const Array* arr, const string& tabName) {
        std::print("\n\n{}:\n", tabName);

        for (const auto& album : *arr) {
            album.Find(albumNameKeys)
                .and_then(&Json::EnsureRef<String>)
                .transform(&String::AsRef)
                .transform([](const auto& name) { return std::println("\t- {}", name), 0; });
        }

        return 0;
    };

    static constexpr auto s_printCollections = [](const Json& content) {
        for (const string tabName : { "Albums", "Videos", "Singles & EPs", "Live performances" }) {
            const auto tab{ content.Search(s_getTab(tabName)) };
            if (!tab) continue;

            (*tab)->Find(tabContentKeys)
                    .and_then(&Json::EnsureRef<Array>)
                    .transform(std::bind_back(s_printAlbumName, tabName));

            if ((*tab)->Find(moreContentButtonKeys))
                std::println("\tMore...");
        }
        return std::monostate{};
    };

    const JsonObject body{
        { "videoId", id },
        { "context", JsonObject{
            { "client", JsonObject{
                { "clientName", "WEB_REMIX" },
                { "clientVersion", std::format("1.{:%Y%m%d}.01.00", std::chrono::system_clock::now()) }
            } }
        } }
    };

    const auto url{ "https://music.youtube.com/youtubei/v1/browse?prettyPrint=false" };

    return NHttp::PostRequest::FromUrl(url, body)
                .and_then(NHttp::Client::Send<Thoth::Http::PostMethod>)
                .and_then(&NHttp::PostResponse::AsJson)

                .and_then(std::bind_back(&Json::FindAndMoveOrError, musicTabKeys))
                .transform(s_printCollections);
}


int main() {

    /* "UCTmoyDN-uokTbzk_xXKcx6w" */
    if (const auto oper{ PrintInfo("UCTmoyDN-uokTbzk_xXKcx6w") }; !oper)
        std::println("{}", oper.error());

    return 0;
}
#include <print>
#include <chrono>

#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/Client.hpp>
#include <Thoth/Utils/Functional.hpp>

std::expected<std::monostate, string> MakeFunctionalRequest(string_view id) {
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
        0,
        "tabRenderer", "content", "sectionListRenderer", "contents" };
    std::array<Key, 2> tabContentKeys{ "musicCarouselShelfRenderer", "contents" };
    std::array<Key, 7> tabTitleKeys{ "musicCarouselShelfRenderer", "header",
                "musicCarouselShelfBasicHeaderRenderer", "title", "runs", 0, "text" };
    std::array<Key, 5> albumNameKeys{ "musicTwoRowItemRenderer", "title", "runs", 0, "text" };
    std::array<Key, 4> moreContentButtonKeys{ "musicCarouselShelfRenderer", "header",
                "musicCarouselShelfBasicHeaderRenderer", "moreContentButton" };
#pragma endregion

    std::string body{ std::format(R"(
{{
    "browseId": "{}",
    "context": {{
        "client": {{
            "clientName": "WEB_REMIX",
            "clientVersion": "1.{:%Y%m%d}.01.00"
        }}
    }}
}}
    )", id, std::chrono::system_clock::now()) };

    const auto contentTabs{ // keeping the json alive
        NHttp::PostRequest::FromUrl("https://music.youtube.com/youtubei/v1/browse?prettyPrint=false", body)
                .transform(NHttp::Client::Send<Thoth::Http::PostMethod>)
                .value_or(std::unexpected{ "Failed to connect." })
                .transform(&NHttp::PostResponse::AsJson)
                .and_then(Utils::ValueOrHof<Json>("Cant convert to json."s))
                .transform(std::bind_back(&Json::FindAndMove, musicTabKeys))
                .and_then(Utils::ValueOrHof<Json>("Json structure mismatch."s))
                .and_then(Utils::ErrorIfNotHof<&Json::IsOf<Array>, Json>("Json structure mismatch."s))
    };
    /* musicTabKeys seams to be [0] every time but just in case you can use this:
     * .transform(std::bind_back(&Json::FindAndMove,
     *     Keys{{ "contents", "singleColumnBrowseResultsRenderer", "tabs" }}))
     *
     * .transform(std::bind_back(&Json::SearchAndMove<>,
     *     [](const Json& tab) -> bool {
     *         if (const auto title{ tab.Find({{ "tabRenderer", "title" }})}; title)
     *             return title->get() == Json{ "Music" }; // std::optional<&T> please 🙏🏾
     *         return false;
     *     }))
     *
     * .transform(std::bind_back(&Json::FindAndMove,
     * Keys{{ "tabRenderer", "content", "sectionListRenderer", "contents" }}))
    **/

    if (!contentTabs)
        return std::unexpected{ contentTabs.error() };

    const auto getTab = [&](const string& name) {
        return [&](const Json& tab) {
            if (const auto title{ tab.Find(tabTitleKeys) }; title)
                return title->get() == Json{ name };
            return false;
        };
    };


    for (const string tabName : { "Albums", "Videos", "Singles & EPs", "Live performances" }) {
        const auto tab{ contentTabs->Search(getTab(tabName)) };
        if (!tab) continue;

        const auto content{
            tab->get().Find(tabContentKeys)
            .and_then(&Utils::NulloptIfNot<&Json::IsOf<Array>, CRef>)
        };
        if (!content) continue;

        std::print("\n\n{}:\n", tabName);

        for (const auto& album : content->get().As<Array>()) {
            album.Find(albumNameKeys)
                .and_then(&Utils::NulloptIfNot<&Json::IsOf<String>, CRef>)
                .transform([](const auto& name){ return name.get().template As<String>().AsRef(); })
                .transform([](const auto& name) { return std::println("\t- {}", name), 0; });
        }

        if (tab->get().Find(moreContentButtonKeys))
                std::println("\tMore...");
    }

    return {};
}


int main() {

    /* "UCTmoyDN-uokTbzk_xXKcx6w" */
    if (const auto oper{ MakeFunctionalRequest("UCRQX-dpFt_osBpH71ItuuvA") }; !oper)
        std::println("{}", oper.error());

    return 0;
}
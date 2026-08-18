#include <print>
#include <chrono>

#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/Client/Client.hpp>

#pragma warning(disable: 4455)

std::expected<std::monostate, Thoth::ThothError> PrintInfo(std::string_view id) {
#pragma region Aliases and Key definitions
    namespace NHttp = Thoth::Http;
    namespace Utils = Thoth::Utils;

    using Thoth::NJson::Key;

    using Thoth::NJson::Json;
    using Thoth::NJson::Array;
    using Thoth::NJson::JsonObject;
    using Thoth::NJson::String;

    using Thoth::ThothError;
    using Thoth::GenericError;


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


#pragma region Lambdas

    static constexpr auto getTab{ [](const std::string& name) {
        return [&](const Json& tab) {
            if (const auto title{ tab.Find(tabTitleKeys) }; title)
                return **title == Json{ name };
            return false;
        };
    } };

    static constexpr auto printAlbumName = [](const Array* arr, const std::string& tabName) {
        std::print("\n\n{}:\n", tabName);

        for (const auto& album : *arr) {
            album.Find(albumNameKeys)
                    .and_then(&Json::EnsureRef<String>)
                    .transform([](const auto& name) { return std::println("\t- {}", *name), 0; });
        }

        return 0;
    };

    static constexpr auto printCollections = [](const Json& content) {
        for (const std::string tabName : { "Albuns", "Videos", "Singles & EPs", "Live performances" }) {
            const auto tab{ content.Search(getTab(tabName)) };
            if (!tab) continue;

            (*tab)->Find(tabContentKeys)
                    .and_then(&Json::EnsureRef<Array>)
                    .transform(std::bind_back(printAlbumName, tabName));

            if ((*tab)->Find(moreContentButtonKeys))
                std::println("\tMore...");
        }
        return std::monostate{};
    };

    static constexpr auto processRequest = [](NHttp::PostResponse&& req) -> std::expected<NHttp::PostResponse, ThothError> {
        switch (req.status) {
            case NHttp::StatusCodeEnum::Ok:
                return std::move(req);
            case NHttp::StatusCodeEnum::BadRequest:
                return std::unexpected{ ThothError{ GenericError{ std::format("Bad Request:\n\n{}", req.body) } } };
            default:
                return std::unexpected{ ThothError{ GenericError{ "Invalid Request" } } };
        }
    };

#pragma endregion

    using Clock = std::chrono::system_clock;

    const JsonObject body{
        { "browseId", id },
        { "context", JsonObject{
            { "client", JsonObject{
                { "clientName", "WEB_REMIX" },
                { "clientVersion", std::format("1.{:%Y%m%d}.01.00", Clock::now()) }
            } }
        } }
    };

    const auto url{ "https://music.youtube.com/youtubei/v1/browse?prettyPrint=false" };

    return NHttp::PostRequest::FromUrl(url, body)
                .and_then(NHttp::Client::H_Send())
                .and_then(processRequest)
                .and_then(&NHttp::PostResponse::AsJson<>)

                .and_then(std::bind_back(&Json::FindAndMoveOrError, musicTabKeys))
                .transform(printCollections);
}


int main() {
    std::println("sla");
    /* "UCTmoyDN-uokTbzk_xXKcx6w" */
    if (const auto oper{ PrintInfo("UCTmoyDN-uokTbzk_xXKcx6w") }; !oper)
        std::println("{}", oper.error());


    return 0;
}
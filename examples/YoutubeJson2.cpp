#include <print>
#include <chrono>

#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/Client.hpp>
#include <Thoth/Utils/Functional.hpp>


std::expected<std::monostate, string> MakeRequest() {
    namespace NHttp = Thoth::Http;
    namespace Utils = Thoth::Utils;

    using Json = Thoth::NJson::Json;

    const auto page{
        NHttp::GetRequest::FromUrl("https://www.youtube.com/@ringosheenaofficial/releases")
                .transform(NHttp::Client::Send<>)
                .value_or(std::unexpected{ "Failed to connect." })
    };

    if (!page)
        return std::unexpected{ page.error() };

    const auto jsonStart{
        page->body.substr(page->body.find("ytInitialData = ") + strlen("ytInitialData = "))
    };

    const auto albums{
        Json::Parse(jsonStart, false, false)
                .transform(std::bind_back(&Json::GetAndMove, "contents"))
                .transform(std::bind_back(&Json::GetAndMove, "twoColumnBrowseResultsRenderer"))
                .transform(std::bind_back(&Json::GetAndMove, "tabs"))
                .transform(std::bind_back(&Json::GetAndMove, 3))
                .transform(std::bind_back(&Json::GetAndMove, "tabRenderer"))
                .transform(std::bind_back(&Json::GetAndMove, "content"))
                .transform(std::bind_back(&Json::GetAndMove, "richGridRenderer"))
                .transform(std::bind_back(&Json::GetAndMove, "contents"))
    };

    if (!albums || (*albums)->IsOf<Thoth::NJson::Array>()) {
        for (const auto album: (*albums)->As<Thoth::NJson::Array>()) {
            const auto name{ album.Find({
                { "richItemRenderer","content", "playlistRenderer", "title" }}) };
            if (name)
                std::println("{}", name->get());
        }
    }


    return {};
}


int main() {

    if (const auto err{ MakeRequest() }; !err)
        std::print("{}", err.error());


    return 0;
}
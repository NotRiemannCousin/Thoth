#include <print>
#include <chrono>

#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/Client.hpp>
#include <Thoth/Utils/Functional.hpp>

int main() {
    std::string body{ std::format(R"(
{{
    "video_id": "{}",
    "context": {{
        "client": {{
            "clientName": "WEB_REMIX",
            "clientVersion": "1.{:%Y%m%d}.01.00"
        }}
    }}
}}
)", "hbckxFs-obM", std::chrono::system_clock::now()) };

    namespace NHttp = Thoth::Http;
    namespace Utils = Thoth::Utils;

    using Json = Thoth::NJson::Json;

    const auto json{
        NHttp::PostRequest::FromUrl("https://music.youtube.com/youtubei/v1/player", body)
                .transform(NHttp::Client::Send<Thoth::Http::PostMethod>)
                .value_or(std::unexpected{ "Failed to connect." })
                .transform(&NHttp::PostResponse::AsJson)
                .and_then(Utils::ValueOrHof<Json>(string{ "Cant convert to json." }))
    };

    if (json)
        std::print("{}", json.value());
    else
        std::print("{}", json.error());


    return 0;
}
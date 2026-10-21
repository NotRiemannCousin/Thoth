#include <Test3.hpp>

#include <curl/curl.h>
#include <simdjson.h>
#include <vector>
#include <string>
#include <optional>
#include <format>


//? curllib + simdJSON

//? curl chama isso a cada chunk recebido
static size_t CurlWriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size{ size * nmemb };
    auto* buffer = static_cast<std::string*>(userp);
    buffer->append(static_cast<char*>(contents), total_size);                //> acumula o body no buffer
    return total_size;
}

std::optional<std::vector<std::string>> Test3::GetMembers(size_t id) {
    CURL* curl{ curl_easy_init() };                                          //? handle global do libcurl
    if (!curl) return std::nullopt;

    std::string response_body;
    auto url{ std::format("https://api.discogs.com/artists/{}", id) };       //* monta a URL completa

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());                        //| define a URL (https já liga TLS)
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);        //| função que recebe os chunks
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);               //| passa o buffer pro callback
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "CurlClient");

    CURLcode res{ curl_easy_perform(curl) };                                 //| executa a request (bloqueante)
    long http_code{};
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);             //| lê o status HTTP retornado
    curl_easy_cleanup(curl);                                                 //? libera o handle

    if (res != CURLE_OK || http_code != 200) return std::nullopt;            //| erro de rede ou status != 200

    const simdjson::padded_string json_padded{ response_body };              //? simdjson exige padding SIMD

    try {
        simdjson::ondemand::parser parser;                                   //? parser on-demand (parse lazy, sem DOM)
        auto doc{ parser.iterate(json_padded) };                             //| parse inicial
        std::vector<std::string> names;                                      //> acumulador

        for (auto member : doc["members"]) {                                 //| seleciona ["members"] como array
            std::string_view name_view;
            if (member["name"].get_string(name_view) == simdjson::SUCCESS)   //| tenta extrair "name" como string
                names.emplace_back(name_view);                               //> copia o nome
            else
                names.push_back("<unnamed>");                                //> fallback
        }
        return names;
    } catch (...) {                                                          //| erro inesperado de parse
        return std::nullopt;
    }
}
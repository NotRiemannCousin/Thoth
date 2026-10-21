#include <Test1.hpp>

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <optional>
#include <format>
#include <rapidjson/document.h>


//? httplib + RapidJSON

std::optional<std::vector<std::string>> Test1::GetMembers(size_t id) {
    httplib::Client cli("https://api.discogs.com");                              //? cliente HTTPS
    auto res{ cli.Get(std::format("/artists/{}", id)) };                         //* monta o path e envia GET
    if (!res || res->status != 200) return std::nullopt;                         //| falha sem conexão ou status != 200

    rapidjson::Document d;                                                      //? documento RapidJSON (DOM)
    d.Parse(res->body.c_str());                                                 //| parse in-place (string terminada em NUL)
    if (d.HasParseError() || !d.HasMember("members") || !d["members"].IsArray()) //| valida parse + ["members"]
        return std::nullopt;

    std::vector<std::string> names;                                              //> acumulador
    const auto& members{ d["members"] };
    for (rapidjson::SizeType i{ 0 }; i < members.Size(); ++i) {                  //| itera o array
        const auto& m{ members[i] };
        if (m.IsObject() && m.HasMember("name") && m["name"].IsString())         //| objeto + campo string?
            names.push_back(m["name"].GetString());                              //> copia o nome
        else
            names.push_back("<unnamed>");                                        //> fallback
    }
    return names;
}
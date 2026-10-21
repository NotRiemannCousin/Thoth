#include <Test0.hpp>

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include <optional>
#include <format>


//? httplib + nlohmann

std::optional<std::vector<std::string>> Test0::GetMembers(size_t id) {
    try {
        httplib::Client cli{ "https://api.discogs.com" };             //? cria client HTTPS (o "https://" já liga o TLS)
        auto res{ cli.Get(std::format("/artists/{}", id)) };          //* monta o path e envia GET -> Response
        if (!res || res->status != 200) return std::nullopt;          //| falha se não conectou ou status != 200

        std::vector<std::string> names;                               //> acumulador dos nomes
        auto j{ nlohmann::json::parse(res->body) };                   //| parse do body como JSON (DOM na memória)

        for (const auto& member : j.at("members")) {                   //| seleciona ["members"] (lança se ausente)
            if (member.contains("name") && member["name"].is_string()) //| "name" existe e é string?
                names.push_back(member["name"].get<std::string>());    //> copia o nome
            else
                names.push_back("<unnamed>");                          //> fallback sem nome
        }
        return names;
    } catch (const nlohmann::json::exception&) {                       //| erro de parse/estrutura -> nullopt
        return std::nullopt;
    }
}


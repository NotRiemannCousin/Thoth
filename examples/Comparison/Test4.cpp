#include <Test4.hpp>

#include <Thoth/Http/Client/Client.hpp>
#include <Thoth/NJson/Json.hpp>
#include <functional>
#include <algorithm>
#include <ranges>


using namespace Thoth::Http;
using namespace Thoth::NJson;

namespace rg = std::ranges;
namespace vs = std::views;

std::expected<std::vector<std::string>, Thoth::ThothError> Test4::GetMembers(size_t id) {
    std::string url{ std::format("https://api.discogs.com/artists/{}", id) };

    //? Trabalhando com std::optional
    constexpr auto extractName{ [](Json j) {
        return std::move(j).GetAndMove("name")      //| seleciona ["name"] ("name" pode ser um campo inválido)
                .and_then(&Json::EnsureMov<String>) //| casting pra String do Thoth (Json pode não ser String)
                .transform(rg::to<std::string>())   //* convertendo pra std::string
                .value_or("<unnamed>");             //> retorne "<unnamed>" caso algum passo tenha falhado
    } };

    //? Trabalhando com std::expected
    return GetRequest::FromUrl(std::move(url))                              //| Parsea a URL e cria a Request
            .and_then(Client::H_Send())                                     //| Envia Request, recebe Response
            .and_then(&GetResponse::EnsureSuccess)                          //| Descarta se não for 200
            .and_then(&GetResponse::AsJson<>)                               //| Converte o body
            .and_then(std::bind_back(&Json::GetAndMoveOrError, "members"))  //| Seleciona ["members"]
            .transform(vs::transform(extractName) | rg::to<std::vector>()); //* extrai os nomes e monta o vector
}



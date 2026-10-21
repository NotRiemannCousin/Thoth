#include <Test2.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/json.hpp>
#include <vector>
#include <string>
#include <format>


namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;
namespace json = boost::json;
using tcp = net::ip::tcp;

//? BoostBeast + BoostJSON (cliente HTTPS)

std::expected<std::vector<std::string>, boost::system::error_code> Test2::GetMembers(size_t id) {
    //? Setup: io_context + context TLS (carrega CAs do sistema)
    net::io_context ioc;
    ssl::context ctx{ ssl::context::tlsv12_client };
    ctx.set_default_verify_paths();                                         //| carrega certificados raiz confiáveis

    tcp::resolver resolver{ ioc };
    beast::ssl_stream<beast::tcp_stream> stream{ ioc, ctx };

    boost::system::error_code ec;

    auto const results{ resolver.resolve("api.discogs.com", "443", ec) };   //| resolve o DNS (host -> IP)
    if (ec) return std::unexpected{ ec };

    beast::get_lowest_layer(stream).connect(results, ec);                   //| abre a conexão TCP (camada mais baixa)
    if (ec) return std::unexpected{ ec };

    //| handshake TLS informando o SNI (host virtual)
    if (!SSL_set_tlsext_host_name(stream.native_handle(), "api.discogs.com"))
        return std::unexpected{ boost::system::error_code{
            static_cast<int>(::ERR_get_error()), net::error::get_ssl_category() } };
    stream.handshake(ssl::stream_base::client, ec);
    if (ec) return std::unexpected{ ec };

    auto target{ std::format("/artists/{}", id) };
    http::request<http::string_body> req{ http::verb::get, target, 11 }; //* monta o path da request
    req.set(http::field::host, "api.discogs.com");
    req.set(http::field::user_agent, "BeastClient");

    http::write(stream, req, ec);
    if (ec) return std::unexpected{ ec };                               //| envia a request pela conexão TLS

    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    http::read(stream, buffer, res, ec);
    if (ec || res.result() != http::status::ok) return std::unexpected{ ec }; //| lê a response (precisa ser 200 OK)


    json::value jv{ json::parse(res.body(), ec) };                      //| parse do body como JSON
    if (ec || !jv.is_object()) return std::unexpected{ ec };

    auto const* members{ jv.as_object().if_contains("members") };       //| seleciona ["members"] (deve ser array)
    if (!members || !members->is_array()) return std::unexpected{ ec };

    std::vector<std::string> names;                                     //> extrai os nomes e monta o vector
    for (auto const& item : members->as_array()) {
        if (item.is_object()) {
            auto const* name{ item.as_object().if_contains("name") };
            if (name && name->is_string()) {
                names.push_back(std::string{ name->as_string() });
                continue;
            }
        }
        names.push_back("<unnamed>");
    }

    stream.shutdown(ec);                                                //| fecha a conexão TLS (best-effort)

    return names;
}



#include <print>
#include <chrono>
#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/Client/Client.hpp>

static std::expected<std::monostate, Thoth::ThothError> SaveImage(const std::string_view url) {
    namespace NHttp = Thoth::Http;
    using     FileT = Thoth::Dsa::BinFileOutputRange;

    return NHttp::GetRequest::FromUrl(url)
            .and_then(NHttp::Client::H_SendAsAndParse<FileT>(FileT::H_AsBody({ "./output.jpg" })))
            .transform([](auto){ return std::monostate{}; });
}


int main() {

    constexpr char url[]{ "https://is1-ssl.mzstatic.com/image/thumb/Music/bc/74/6d/mzi.vxwaqkit.jpg/1200x1200bb.jpg" };
    if (const auto oper{ SaveImage(url) }; !oper)
        std::println("{}", oper.error());


    return 0;
}
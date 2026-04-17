#include <print>
#include <chrono>
#pragma warning(disable: 4455)

#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/Client.hpp>
#include <Thoth/Utils/Functional.hpp>

std::expected<std::monostate, Thoth::Http::RequestError> SaveImage(std::string_view url) {
    namespace NHttp = Thoth::Http;
    using FileT = Thoth::Dsa::BinFileOutputRange;

    Thoth::Dsa::FileBuilderParams sla{ "./output.jpg" };
    return NHttp::GetRequest::FromUrl(url)
            .and_then(NHttp::Client::H_SendAndRecvAsInto<FileT>(FileT::H_AsBody(sla)))
            .transform([](auto){ return std::monostate{}; });
}


int main() {

    constexpr char url[]{ "https://is1-ssl.mzstatic.com/image/thumb/Music/bc/74/6d/mzi.vxwaqkit.jpg/1200x1200bb.jpg" };
    if (const auto oper{ SaveImage(url) }; !oper)
        std::println("{}", oper.error());


    return 0;
}
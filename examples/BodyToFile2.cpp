#include <print>
#include <chrono>
#pragma warning(disable: 4455)

#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/Client.hpp>
#include <Thoth/Utils/Functional.hpp>

std::expected<std::monostate, Thoth::Http::RequestError> SavePdf(std::string_view url) {
    namespace NHttp = Thoth::Http;
    using FileT = Thoth::Dsa::BinFileOutputRange;

    return NHttp::GetRequest::FromUrl(url)
            .and_then(NHttp::Client::H_SendAndRecvAsInto<FileT>(FileT::H_AsBody({ "./pg-17-never-forget.pdf" })))
            .transform([](auto){ return std::monostate{}; });
}


int main() {

    constexpr char url[]{ "https://www.andifes.org.br/wp-content/files_flutter/1379600228mercadante.pdf" };
    if (const auto oper{ SavePdf(url) }; !oper)
        std::println("{}", oper.error());


    return 0;
}
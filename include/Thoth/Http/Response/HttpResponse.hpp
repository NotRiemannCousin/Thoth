#pragma once
#include <Thoth/Http/Response/HttpStatusCodeEnum.hpp>
#include <Thoth/Json/Json.hpp>


namespace Thoth::Http {
    template<HttpMethodConcept Method>
    struct HttpResponse {
        HttpVersion version{};
        HttpStatusCodeEnum status{};
        string statusMessage{};
        HttpHeaders headers{};
        string body{};

        friend HttpClient;

        std::optional<Json::Json> AsJson() const;
    private:

        HttpResponse(HttpVersion version, HttpStatusCodeEnum status,
                string statusMessage, HttpHeaders headers, string body);
    };
}



#include <Thoth/Http/Response/HttpResponse.tpp>


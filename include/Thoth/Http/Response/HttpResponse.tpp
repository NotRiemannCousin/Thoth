#pragma once

namespace Thoth::Http {
    template<HttpMethodConcept Method>
    HttpResponse<Method>::HttpResponse(HttpVersion version, HttpStatusCodeEnum status, string&& statusMessage,
            HttpHeaders&& headers, string&& body) : version{ version },  status{ status },
            statusMessage{ std::move(statusMessage) },  headers{ std::move(headers) },
            body{ std::move(body) } { }

    template<HttpMethodConcept Method>
    std::optional<NJson::Json> HttpResponse<Method>::AsJson() const {
        return NJson::Json::Parse(body);
    }
}

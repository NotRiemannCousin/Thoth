#pragma once

namespace Thoth::Http {
    template<MethodConcept Method>
    Response<Method>::Response(Version version, StatusCodeEnum status, string&& statusMessage,
            Headers&& headers, string&& body) : version{ version },  status{ status },
            statusMessage{ std::move(statusMessage) },  headers{ std::move(headers) },
            body{ std::move(body) } { }

    template<MethodConcept Method>
    std::optional<NJson::Json> Response<Method>::AsJson() const {
        return NJson::Json::Parse(body);
    }
}

#pragma once

namespace Thoth::Http {
    template<MethodConcept Method>
    Response<Method>::Response(VersionEnum version, StatusCodeEnum status, string&& statusMessage,
            Headers&& headers, string&& body) : version{ version },  status{ status },
            statusMessage{ std::move(statusMessage) },  headers{ std::move(headers) },
            body{ std::move(body) } { }

    template<MethodConcept Method>
    std::expected<NJson::Json, string> Response<Method>::AsJson() const {
        return NJson::Json::Parse(body);
    }

    template<MethodConcept Method>
    bool Response<Method>::Successful() const {
        return GetStatusType(status) == StatusTypeEnum::SUCCESSFUL;
    }

    template<MethodConcept Method>
    string Response<Method>::MoveBody() && {
        return std::move(body);
    }
}

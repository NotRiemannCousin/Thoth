#pragma once
#include <Thoth/Http/Response/StatusCodeEnum.hpp>
#include <Thoth/Http/Methods/PostMethod.hpp>
#include <Thoth/Http/Methods/GetMethod.hpp>
#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Headers.hpp>
#include <Thoth/NJson/Json.hpp>


namespace Thoth::Http {
    template<MethodConcept Method = GetMethod>
    struct Response {
        using MethodType = Method;

        VersionEnum version{};
        StatusCodeEnum status{};
        string statusMessage{};
        Headers headers{};
        string body{};

        friend struct Client; // who construct it

        [[nodiscard]] std::optional<NJson::Json> AsJson() const;
    private:

        Response(VersionEnum version, StatusCodeEnum status,
                string&& statusMessage, Headers&& headers, string&& body);
    };

    using GetResponse  = Response<>;
    using PostResponse = Response<PostMethod>;
}



#include <Thoth/Http/Response/Response.tpp>


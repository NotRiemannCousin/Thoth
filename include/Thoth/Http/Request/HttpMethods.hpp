#pragma once
#include <string>
#include <variant>


namespace Thoth::Http {
    // RFC 7231 and RFC 5789
    enum class HttpMethodsEnum{
        GET,
        HEAD,
        POST,
        PUT,
        DELETE,
        CONNECT,
        OPTIONS,
        TRACE,
        PATCH
    };

    using CustomHttpMethod = std::string;
    using HttpMethod = std::variant<HttpMethodsEnum, CustomHttpMethod>;
}

#pragma once
#include <Hermes/Socket/Sync/ServerSocket.hpp>

#include <Thoth/Http/ExchangeError.hpp>
#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>

namespace Thoth::Http {

    struct ServerConnection : std::variant<Hermes::RawTcpServer, Hermes::RawTlsServer> {};


    struct Server {
        template <class BodyType, MethodConcept... RequestTypes>
            requires (sizeof...(RequestTypes) > 0)
        ExchangeResult<std::variant<Request<RequestTypes, BodyType>...>> Receive(ServerConnection& conn);

    };
}

#include <Thoth/Http/Server.tpp>
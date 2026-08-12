#pragma once
#include <variant>

#include <Hermes/Socket/Sync/ServerSocket.hpp>

#include <Thoth/Http/ExchangeError.hpp>
#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>

namespace Thoth::Http {

    struct ServerConnection{
        using SocketType = std::variant<Hermes::RawTcpServer, Hermes::RawTlsServer>;
        SocketType socket;

        template<class T>
        auto Send(const T& data);
        void Close();
        void Abort();
    };


    struct Server {
        template<class BodyType, MethodConcept... RequestTypes>
            requires (sizeof...(RequestTypes) > 0)
        ExchangeResult<std::variant<Request<RequestTypes, BodyType>...>> Receive(ServerConnection& conn);

    };
}

#include <Thoth/Http/Server.tpp>

namespace Thoth::Http {
    static_assert(ConnectionConcept<ServerConnection>);
}
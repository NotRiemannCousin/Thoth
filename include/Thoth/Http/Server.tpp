#pragma once 
#include <Thoth/Http/_base/Http1.hpp>

namespace Thoth::Http {

    template<class T>
    auto ServerConnection::Send(const T& data) {
        return std::visit([&](auto& sock) { return sock.Send(data); }, socket);
    }

    inline void ServerConnection::Close() {
        std::visit([](auto& sock) { sock.Close(); }, socket);
    }

    inline void ServerConnection::Abort() {
        std::visit([](auto& sock) { sock.Abort(); }, socket);
    }




    template<class BodyType, MethodConcept... RequestTypes>
        requires (sizeof...(RequestTypes) > 0)
    ThothResult<std::variant<Request<RequestTypes, BodyType>...>> Server::Receive(ServerConnection& conn) {
        ThothResult<std::variant<Request<RequestTypes, BodyType>...>> result;

        static constexpr auto createHead{ [](auto&& socket) {
            return details_::Http1::ParseResponseLine(std::move(socket));
        } };

        auto lineRes{ std::visit(createHead, conn) };
        if (!lineRes) return std::unexpected{ lineRes.error() };

        const auto& [method, stage]{ *lineRes };

        (std::invoke([&] {
            if (!result && RequestTypes::MethodName() == method)
                result.emplace(Request<RequestTypes, BodyType>{});
        }), ...);

        return result;
    }
}


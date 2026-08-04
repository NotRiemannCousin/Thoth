#pragma once 
#include <Thoth/Http/_base/Http11.hpp>

namespace Thoth::Http {

    template <class BodyType, MethodConcept... RequestTypes>
        requires (sizeof...(RequestTypes) > 0)
    ExchangeResult<std::variant<Request<RequestTypes, BodyType>...>> Server::Receive(ServerConnection& conn) {
        ExchangeResult<std::variant<Request<RequestTypes, BodyType>...>> result;

        static constexpr auto createHead{ [](auto&& socket) {
            return details_::Http11::ParseResponseLine(std::move(socket));
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


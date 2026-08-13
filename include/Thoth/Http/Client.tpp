#pragma once
#include <Thoth/Http/_base/Http1.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/ExchangeError.hpp>
#include <Hermes/Utils/Overloads.hpp>
#include <string_view>

#pragma region Macros
#pragma push_macro("ASSERT_OR_RET_ERROR")
#pragma push_macro("ASSERT_OR_RET_EXC_ERROR")
#undef ASSERT_OR_RET_ERROR
#undef ASSERT_OR_RET_EXC_ERROR

#define ASSERT_OR_RET_ERROR(cond, error) do {       \
    if (!(cond)) return std::unexpected{ (error) }; \
} while (0)
#define ASSERT_OR_RET_EXC_ERROR(cond, error) ASSERT_OR_RET_ERROR(cond, ExchangeError{ (error) })

#pragma endregion

#pragma warning(disable: 4455)

namespace Thoth::Http {
    template<class T>
    auto ClientConnection::Send(const T& data) {
        return std::visit([&](auto& sock) { return sock.Send(data); }, socket);
    }
    inline void ClientConnection::Close() {
        std::visit([](auto& sock) { sock.Close(); }, socket);
    }
    inline void ClientConnection::Abort() {
        std::visit([](auto& sock) { sock.Abort(); }, socket);
    }




    template<MethodConcept Method, BodyConcept Body>
        requires std::default_initializable<Body>
    auto Client::Send(Request<Method, Body> request, ClientOptions opts) -> ExpResponse<Method, Body> {
        return SendAsAndParse<Method, Body, Body>(
            request,[](const ResponseHead&) -> std::expected<Body, ExchangeError> { return {}; }, opts
        );
    }

    template<MethodConcept Method, BodyConcept Body, class F>
        requires ResponseBodyFactoryConcept<F, Body>
    auto Client::SendAndParse(Request<Method, Body> request, F&& bodyFactory, ClientOptions opts) -> ExpResponse<Method, Body> {
        return SendAsAndParse<Method, Body, Body>(request, std::forward<F>(bodyFactory), opts);
    }


    template<MethodConcept Method, ReadableBodyConcept RequestBody, WritableBodyConcept ResponseBody>
        requires std::default_initializable<ResponseBody>
    auto Client::SendAs(Request<Method, RequestBody> request, ClientOptions opts) -> ExpResponse<Method, ResponseBody> {
        return SendAsAndParse<Method, RequestBody, ResponseBody>(
            request, [](const ResponseHead&) -> std::expected<ResponseBody, ExchangeError> { return {}; }, opts
        );
    }


    template<MethodConcept Method, ReadableBodyConcept RequestBody, WritableBodyConcept ResponseBody, class F>
        requires ResponseBodyFactoryConcept<F, ResponseBody>
    auto Client::SendAsAndParse(
        Request<Method, RequestBody> request, F&& bodyFactory, ClientOptions opts) -> ExpResponse<Method, ResponseBody> {

        static constexpr auto toExchangeError{ [](const auto err) {
            return ExchangeError{ err };
        } };

        const auto scheme{ request.url.GetScheme() };
        ASSERT_OR_RET_EXC_ERROR(scheme == "http" || scheme == "https", GenericError{ "Invalid scheme" });

        const auto auth{ request.url.GetAuthority() };
        ASSERT_OR_RET_EXC_ERROR(auth, GenericError{ "No authority provided" });

        const auto getDefaultPort{ [&] { return GetDefaultPort(request.url.GetScheme()); } };
        const auto port{ auth->port.or_else(getDefaultPort) };
        ASSERT_OR_RET_EXC_ERROR(port, GenericError{ "No port provided" });

        const std::string hostname{
            std::visit(Hermes::Utils::Overloaded{
                [](const Hermes::IpAddress addr) { return std::format("{}", addr); },
                [](const std::string_view  addr) { return std::string{ addr };     },
            }, auth->host)
        };

        ClientJanitor& janitor{ ClientJanitor::Instance() };

        const auto establishConnection{ [&](Hermes::IpEndpoint&& endpoint) {

#pragma region create socket

            const auto getSocketFromPool{ [&]() -> std::optional<SocketPtr> {
                std::lock_guard lock{ janitor.poolMutex };

                const decltype(janitor.connectionPool)::iterator connContainerIt{ janitor.connectionPool.find(endpoint) };

                if (connContainerIt == janitor.connectionPool.end() || connContainerIt->second.empty())
                    return std::nullopt;

                auto infoPtr{ std::move(connContainerIt->second.back()) };
                connContainerIt->second.pop_back();

                return std::move(infoPtr);
            } };

            const auto createNewSocket{ [&]() -> std::optional<SocketPtr> {
                using RawData = Hermes::DefaultSocketData<>;
                using TlsData = Hermes::TlsSocketData<>;
                using TlsSocket = Hermes::RawTlsClient;
                using RawSocket = Hermes::RawTcpClient;

                Hermes::DefaultConnectPolicy<>::Options connOpts{ .connectionTimeout = opts.connectionTimeout };

                if (scheme == "https") {
                    Hermes::TlsConnectPolicy<>::Options tlsConnOpts{
                        connOpts                    , opts.handshakeTimeout,
                        opts.ignoreCertificateErrors, opts.requestMutualAuth,
                    };
                    if (auto res{ TlsSocket::Connect( TlsData{ endpoint, hostname }, tlsConnOpts) })
                        return std::make_shared<ClientConnection>(std::move(*res));
                    return std::nullopt;
                }

                if (auto res{ RawSocket::Connect( RawData{ endpoint }, connOpts) })
                    return std::make_shared<ClientConnection>(std::move(*res));
                return std::nullopt;
            } };

            const auto cleanupSocket{ [&](std::pair<SocketPtr, Response<Method, ResponseBody>> val) {
                std::lock_guard lock{ janitor.poolMutex };

                static Headers::HeaderValue closeConnectionVal{ "close" };
                static Headers::HeaderValue keepAliveConnectionVal{ "keep-alive" };

                const auto connectionHeader{
                    *val.second.headers.Get("connection")
                            .value_or(val.second.version == VersionEnum::HTTP1_0
                                ? &closeConnectionVal
                                : &keepAliveConnectionVal)
                };

                if (val.first != nullptr && connectionHeader != closeConnectionVal)
                    janitor.connectionPool[endpoint].emplace_back(std::move(val.first));

                return std::move(val.second);
            } };

#pragma endregion

            auto infoPtr{ getSocketFromPool().or_else(createNewSocket).value_or(nullptr) };

#pragma region check and send

            const auto isSocketValid{ [&]() -> Hermes::ConnectionResultOper {
                ASSERT_OR_RET_ERROR(infoPtr, ConnectionErrorEnum::ConnectionFailed);
                return std::monostate{};
            } };

            const auto sendRequest{ [&](std::monostate) -> std::expected<SocketPtr, ExchangeError> {
                request.headers.Add("host", hostname);

                details_::Http1::PrepareBodyHeaders(request.headers, request.body);

                auto headRes{ details_::Http1::SendMessageHead<Method, RequestHead>(*infoPtr, request) };
                ASSERT_OR_RET_ERROR(headRes, headRes.error());

                auto bodyRes{ details_::Http1::SendBody(*infoPtr, request.body) };
                ASSERT_OR_RET_ERROR(bodyRes, bodyRes.error());

                return std::move(infoPtr);
            } };

#pragma endregion

            return isSocketValid()
                    .transform_error(toExchangeError)
                    .and_then(sendRequest)
                    .and_then(std::bind_back(ParseHttp1_<Method, ResponseBody, F>, std::forward<F>(bodyFactory)))
                    .transform(cleanupSocket);
        } };


        return Hermes::IpEndpoint::TryResolve(hostname, std::string{ request.url.GetScheme() })
                .transform_error(toExchangeError)
                .and_then(establishConnection);
    }

    constexpr auto Client::H_Send(ClientOptions opts) {
        return [opts]<MethodConcept Method, BodyConcept Body>(Request<Method, Body> request) {
            return Send<Method, Body>(request, opts);
        };
    }


    template<class F>
    auto Client::H_SendAndParse(F&& bodyFactory, ClientOptions opts) {
        return [factory{ std::forward<F>(bodyFactory) }, opts]<MethodConcept Method, BodyConcept Body>(Request<Method, Body> request) mutable {
            return SendAndParse<Method, Body>(std::move(request), std::forward<F>(factory), opts);
        };
    }

    template<WritableBodyConcept ResponseBody>
    constexpr auto Client::H_SendAs(ClientOptions opts) {
        return [opts]<MethodConcept Method, ReadableBodyConcept RequestBody>(Request<Method, RequestBody> request) {
            return SendAs<Method, RequestBody, ResponseBody>(request, opts);
        };
    }

    template<WritableBodyConcept ResponseBody, class F>
        requires ResponseBodyFactoryConcept<F, ResponseBody>
    auto Client::H_SendAsAndParse(F&& bodyFactory, ClientOptions opts) {
        return [factory{ std::forward<F>(bodyFactory) }, opts]<MethodConcept Method, ReadableBodyConcept RequestBody>(Request<Method, RequestBody> request) {
            return SendAsAndParse<Method, RequestBody, ResponseBody>(std::move(request), std::move(factory), opts);
        };
    }


    template<MethodConcept Method, WritableBodyConcept ResponseBody, class F>
        requires ResponseBodyFactoryConcept<F, ResponseBody>
    std::expected<std::pair<Client::SocketPtr, Response<Method, ResponseBody>>, ExchangeError> Client::ParseHttp1_(
        SocketPtr infoPtr, F&& bodyFactory) {

        const auto forwardBoth{ [&infoPtr](Response<Method, ResponseBody>&& response) {
            return std::pair<SocketPtr, Response<Method, ResponseBody>>{ std::move(infoPtr), std::move(response) };
        } };

        auto createResponse{ [bFactory = std::forward<F>(bodyFactory)](auto&& sock) mutable {
            return details_::Http1::BuildResponse<Method, ResponseBody>(
                sock.template RecvStream<char>(),
                std::forward<F>(bFactory)
            );
        } };

        return std::visit(createResponse, infoPtr->socket)
                .transform(forwardBoth);
    }
}

#pragma warning(default: 4455)
#pragma pop_macro("ASSERT_OR_RET_EXC_ERROR")
#pragma pop_macro("ASSERT_OR_RET_ERROR")

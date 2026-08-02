#pragma once
#include <Thoth/Http/_base/Http11.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/ExchangeError.hpp>
#include <Hermes/Utils/Overloads.hpp>
#include <string_view>
#include <bit>

#pragma region Macros
#pragma push_macro("ASSERT_OR_RET_ERROR")
#pragma push_macro("ASSERT_OR_RET_EXC_ERROR")
#pragma push_macro("HTTP11_FORWARD")
#undef ASSERT_OR_RET_ERROR
#undef ASSERT_OR_RET_EXC_ERROR
#undef HTTP11_FORWARD

#define ASSERT_OR_RET_ERROR(cond, error) do {       \
    if (!(cond)) return std::unexpected{ (error) }; \
} while (0)
#define ASSERT_OR_RET_EXC_ERROR(cond, error) ASSERT_OR_RET_ERROR(cond, ExchangeError{ (error) })
#define HTTP11_FORWARD(methodName) ([](auto stage) { return details_::Http11::methodName(std::move(stage)); })

#pragma endregion

#pragma warning(disable: 4455)

namespace Thoth::Http {
    template<MethodConcept Method, BodyConcept Body>
        requires std::default_initializable<Body>
    auto Client::Send(Request<Method, Body> request) -> ExpResponse<Method, Body> {
        return SendAsAndParse<Method, Body, Body>(
            request,[](const ResponseHead&) -> std::expected<Body, ExchangeError> { return {}; }
        );
    }

    template<MethodConcept Method, BodyConcept Body, class F>
        requires ResponseBodyFactoryConcept<F, Body>
    auto Client::SendAndParse(Request<Method, Body> request, F&& bodyFactory) -> ExpResponse<Method, Body> {
        return SendAsAndParse<Method, Body, Body>(request, std::forward<F>(bodyFactory));
    }


    template<MethodConcept Method, ReadableBodyConcept RequestBody, WritableBodyConcept ResponseBody>
        requires std::default_initializable<ResponseBody>
    auto Client::SendAs(Request<Method, RequestBody> request) -> ExpResponse<Method, ResponseBody> {
        return SendAsAndParse<Method, ReadableBodyConcept, ResponseBody>(
            request, [](const ResponseHead&) -> std::expected<ResponseBody, ExchangeError> { return {}; }
        );
    }


    template<MethodConcept Method, ReadableBodyConcept RequestBody, WritableBodyConcept ResponseBody, class F>
        requires ResponseBodyFactoryConcept<F, ResponseBody>
    auto Client::SendAsAndParse(
        Request<Method, RequestBody> request, F&& bodyFactory) -> ExpResponse<Method, ResponseBody> {

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

        const auto establishConnection = [&](Hermes::IpEndpoint&& endpoint) {

#pragma region create socket

            const auto getSocketFromPool = [&]() -> std::optional<SocketPtr> {
                std::lock_guard lock{ janitor.poolMutex };

                const decltype(janitor.connectionPool)::iterator connContainerIt{ janitor.connectionPool.find(endpoint) };

                if (connContainerIt == janitor.connectionPool.end() || connContainerIt->second.empty())
                    return std::nullopt;

                auto infoPtr{ std::move(connContainerIt->second.back()) };
                connContainerIt->second.pop_back();

                return std::move(infoPtr);
            };

            const auto createNewSocket = [&]() -> std::optional<SocketPtr> {
                auto newSocketResult{ Hermes::RawTlsClient::Connect(Hermes::TlsSocketData{ endpoint, hostname }) };
                if (!newSocketResult)
                    return std::nullopt;

                return std::make_shared<Socket>(std::move(*newSocketResult));
            };

            const auto cleanupSocket = [&](std::pair<SocketPtr, Response<Method, ResponseBody>> val) {
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
            };

#pragma endregion

            auto infoPtr{ getSocketFromPool().or_else(createNewSocket) };

#pragma region check and send

            const auto isSocketValid = [&]() -> Hermes::ConnectionResultOper {
                ASSERT_OR_RET_ERROR(infoPtr, ConnectionErrorEnum::ConnectionFailed);
                return std::monostate{};
            };

            const auto sendRequest = [&](std::monostate) -> std::expected<SocketPtr, ExchangeError> {
                request.headers.Add("host", hostname);

                details_::Http11::PrepareBodyHeaders(request.headers, request.body);

                auto headRes{ details_::Http11::SendRequestLineAndHeaders(
                    (*infoPtr)->socket, Method::MethodName(), request.url, request.version, request.headers
                ) };
                ASSERT_OR_RET_ERROR(headRes, headRes.error());

                auto bodyRes{ details_::Http11::SendBody((*infoPtr)->socket, request.body) };
                ASSERT_OR_RET_ERROR(bodyRes, bodyRes.error());

                return std::move(*infoPtr);
            };

            const auto toExchangeError = [](const auto err) -> ExchangeError {
                return ExchangeError{ err };
            };

#pragma endregion

            return isSocketValid()
                    .transform_error(toExchangeError)
                    .and_then(sendRequest)
                    .and_then(std::bind_back(ParseHttp11_<Method, ResponseBody, F>, std::forward<F>(bodyFactory)))
                    .transform(cleanupSocket);
        };

        const auto toExchangeError = [](const auto err) {
            return ExchangeError{ err };
        };


        return Hermes::IpEndpoint::TryResolve(hostname, std::string{ request.url.GetScheme() })
                .transform_error(toExchangeError)
                .and_then(establishConnection);
    }

    constexpr auto Client::H_Send() {
        return []<MethodConcept Method, BodyConcept Body>(Request<Method, Body> request) {
            return Send<Method, Body>(request);
        };
    }


    template<class F>
    auto Client::H_SendAndParse(F&& bodyFactory) {
        return [&]<MethodConcept Method, BodyConcept Body>(Request<Method, Body> request) {
            return SendAndParse<Method, Body>(request, std::forward<F>(bodyFactory));
        };
    }

    template<WritableBodyConcept ResponseBody>
    constexpr auto Client::H_SendAs() {
        return []<MethodConcept Method, ReadableBodyConcept RequestBody>(Request<Method, RequestBody> request) {
            return SendAs<Method, RequestBody, ResponseBody>(request);
        };
    }

    template<WritableBodyConcept ResponseBody, class F>
        requires ResponseBodyFactoryConcept<F, ResponseBody>
    auto Client::H_SendAsAndParse(F&& bodyFactory) {
        return [&]<MethodConcept Method, ReadableBodyConcept RequestBody>(Request<Method, RequestBody> request) {
            return SendAsAndParse<Method, RequestBody, ResponseBody>(request, std::forward<F>(bodyFactory));
        };
    }


    template<MethodConcept Method, WritableBodyConcept ResponseBody, class F>
        requires ResponseBodyFactoryConcept<F, ResponseBody>
    std::expected<std::pair<Client::SocketPtr, Response<Method, ResponseBody>>, ExchangeError> Client::ParseHttp11_(
        SocketPtr infoPtr, F&& bodyFactory) {

        using StreamType = decltype(infoPtr->socket.RecvStream<char>());
        using ParseCompleteStage = details_::ParseCompleteStage<StreamType, ResponseBody>;

        const auto createResponseStream = [&]() -> std::expected<details_::ResponseParseStage<StreamType>, ExchangeError> {
            return details_::ResponseParseStage<StreamType>{ ResponseHead{}, infoPtr->socket.RecvStream<char>() };
        };

        const auto initializeBody = [&](details_::ResponseParseStage<StreamType> stage) -> std::expected<ParseCompleteStage, ExchangeError> {
            auto bodyExp{ std::invoke(bodyFactory, stage.data) };

            if (!bodyExp) return std::unexpected{ bodyExp.error() };

            return ParseCompleteStage{
                { std::move(stage.data), std::move(stage.stream) },
                std::move(*bodyExp)
            };
        };

        const auto createObject = [&](ParseCompleteStage&& stage) {
            return std::pair{
                    std::move(infoPtr),
                    Response<Method, ResponseBody>{
                        stage.data.version, stage.data.status, std::move(stage.data.statusMessage),
                        std::move(stage.data.headers),
                        std::move(stage.body)
                    }
                };
        };

        return createResponseStream()
                .and_then(HTTP11_FORWARD(ParseResponseLine))
                .and_then(HTTP11_FORWARD(ParseHeaders))
                .and_then(initializeBody)
                .and_then(HTTP11_FORWARD(ParseBody))
                .transform(createObject);
    }
}

#pragma warning(default: 4455)
#pragma pop_macro("HTTP11_FORWARD")
#pragma pop_macro("ASSERT_OR_RET_EXC_ERROR")
#pragma pop_macro("ASSERT_OR_RET_ERROR")

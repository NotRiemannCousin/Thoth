#pragma once
#include <Thoth/Http/Response/Response.hpp>
#include <Hermes/Utils/UntilMatch.hpp>
#include <string_view>
#pragma warning(disable: 4455)

namespace Thoth::Http {
    template<MethodConcept Method>
    expected<Response<Method>, string> Client::Send(Request<Method> request) {
        ClientJanitor& janitor{ ClientJanitor::Instance() };

        const auto s_establishConnection = [&](Hermes::IpEndpoint&& endpoint) {
            const auto s_getSocketFromPool = [&]() -> std::optional<SocketPtr> {
                std::lock_guard lock{ janitor.poolMutex };

                const decltype(janitor.connectionPool)::iterator connContainerIt{ janitor.connectionPool.find(endpoint) };

                if (connContainerIt == janitor.connectionPool.end())
                    return std::nullopt;

                const auto infoPtr{ std::move(connContainerIt->second.back()) };
                connContainerIt->second.pop_back();

                return std::move(infoPtr);
            };

            const auto s_createNewSocket = [&]() -> std::optional<SocketPtr> {
                auto newSocketResult{ Hermes::RawTlsClient::Connect(Hermes::TlsSocketData{ endpoint, request.url.host }) };
                if (!newSocketResult)
                    return std::nullopt;

                return std::make_shared<Socket>(std::move(*newSocketResult));
            };


            SocketPtr infoPtr;
            const auto s_cleanupSocket = [&](auto&& val) {
                std::lock_guard lock{ janitor.poolMutex };
                janitor.connectionPool[endpoint].push_back(std::move(infoPtr));

                return std::move(val);
            };

            try {
                infoPtr = s_getSocketFromPool().or_else(s_createNewSocket).value();

                const auto s_sendRequest = [&]() -> Hermes::ConnectionResult<SocketPtr> {
                    request.headers.Add("host", request.url.host);
                    request.headers.Add("content-length", to_string(request.body.size()));

                    const string_view path{ request.url.path.empty() ? string_view{ "/" } : request.url.path };
                    const auto& query { request.url.query };
                    const auto versionStr{ VersionToString(request.version) };

                    const auto requestStr{ std::format(
                        "{} {}?{} {}\r\n"
                        "{}\r\n"
                        "\r\n"
                        "{}",
                        Method::MethodName(), path, query, versionStr,
                        request.headers,
                        request.body
                    ) };

                    const auto [_, sendRes]{ infoPtr->socket.Send(requestStr) };

                    return sendRes
                            .transform([&](auto){ return std::move(infoPtr); });
                };

                const auto s_connErrToStr = [](auto) -> string {
                    return "Can't connect to endpoint";
                };



                return s_sendRequest()
                        .transform_error(s_connErrToStr)
                        .and_then(_ParseHttp11<Method>)
                        .transform(s_cleanupSocket);
            } catch (...) {
                std::ignore = s_cleanupSocket(0);
                throw;
            }
        };

        const auto s_ConnectionErrorToStr = [](auto) {
            return string{ "DNS Resolution Failed" };
        };


        return Hermes::IpEndpoint::TryResolve(request.url.host, to_string(request.url.port))
                    .transform_error(s_ConnectionErrorToStr)
                    .and_then(s_establishConnection);
    }

    template<MethodConcept Method>
    expected<Response<Method>, string> Client::_ParseHttp11(SocketPtr infoPtr) {
        namespace rg = std::ranges;
        namespace vs = std::views;

        using namespace std::literals;

        struct ParseStage {
            HttpData data{};
            decltype(infoPtr->socket.RecvRange<char>()) stream;
        };

        using ParseResult = std::expected<ParseStage, string>;

        const auto s_createResponseStream = [&]() -> ParseResult {
            return ParseStage{ HttpData{}, infoPtr->socket.RecvRange<char>() };
        };

        const auto s_fillResponseLine = [&](ParseStage&& info) -> ParseResult {
            if (!rg::starts_with(info.stream, "HTTP/1."sv))
                return std::unexpected{ "Invalid response" };

            switch (*info.stream.begin()) {
                case '0': info.data.version = VersionEnum::HTTP1_0; break;
                case '1': info.data.version = VersionEnum::HTTP1_1; break;
                default: return std::unexpected{ "Invalid version" };
            }
            ++info.stream.begin();

            const auto arr{ Hermes::Utils::CopyTo<array<char, 5>>(info.stream) };

            if (arr[0] != ' ' || !isdigit(arr[1]) || !isdigit(arr[2]) || !isdigit(arr[3]) || arr[4] != ' ')
                return std::unexpected{ "Invalid response" };

            info.data.status = static_cast<StatusCodeEnum>((arr[1] - '0') * 100 + (arr[2] - '0') * 10 + (arr[3] - '0'));
            info.data.statusMessage = info.stream | Hermes::Utils::UntilMatch("\r\n"sv) | rg::to<string>();

            return std::move(info);
        };

        const auto s_fillHeaders = [&](ParseStage&& info) -> ParseResult {
            auto rawHeaders{ info.stream | Hermes::Utils::UntilMatch("\r\n\r\n"sv) };
            const auto headersParseRes{ Headers::Parse(rawHeaders) };

            if (!headersParseRes)
                return std::unexpected{ "Invalid headers" };
            info.data.headers = *headersParseRes;
            return std::move(info);
        };

        const auto s_fillBody = [&](ParseStage&& info) -> ParseResult {
            const auto s_hasSizedLength = [&](const Headers::HeaderValue* contentSizeHeader) -> ParseResult {
                size_t contentSize;

                auto [_, ec] = std::from_chars(
                    contentSizeHeader->data(),
                    contentSizeHeader->data() + contentSizeHeader->size(),
                    contentSize
                );

                if (ec != std::errc())
                    return std::unexpected{ "Invalid response" };

                info.data.body = info.stream | vs::take(contentSize) | rg::to<string>();

                return std::move(info);
            };

            const auto s_hasTransferEncoding = [&]() -> std::optional<ParseResult> {
                if (info.data.version == VersionEnum::HTTP1_0)
                    return std::unexpected{ "HTTP/1.0 needs content-length" };

                static string defaultValue{ "chunked" };
                auto transferEncoding{ *info.data.headers.Get("transfer-encoding")
                        .value_or(&defaultValue) };

                if (transferEncoding == "chunked") {
                    string chunkLengthStr;
                    int chunkLength;

                    info.data.body.reserve(0x4000);

                    do {
                        chunkLengthStr.clear();
                        rg::copy(info.stream | Hermes::Utils::UntilMatch("\r\n"sv), std::back_inserter(chunkLengthStr));
                        auto [_, ec] {
                            std::from_chars( chunkLengthStr.data(), chunkLengthStr.data() + chunkLengthStr.size(),
                        chunkLength, 16)
                        };

                        if (ec != std::errc())
                            return std::unexpected{ "Invalid response" };


                        info.data.body.append_range(info.stream | vs::take(chunkLength));

                        if (!rg::starts_with(info.stream, "\r\n"sv))
                            return std::unexpected{ "Invalid response" };

                    } while (chunkLength != 0);
                }

                return std::move(info);
            };


            return info.data.headers.Get("content-length")
                    .transform(s_hasSizedLength)
                    .or_else(s_hasTransferEncoding)
                    .value();
        };

        const auto s_createObject = [](ParseStage&& info) -> Response<Method> {
            return Response<Method>{ info.data.version, info.data.status, std::move(info.data.statusMessage),
                    std::move(info.data.headers), std::move(info.data.body) };
        };


        return s_createResponseStream()
                .and_then(s_fillResponseLine)
                .and_then(s_fillHeaders)
                .and_then(s_fillBody)
                .transform(s_createObject);

    }
};


#pragma once
#include <Thoth/Http/Response/Response.hpp>
#include <Hermes/Utils/UntilMatch.hpp>
#include <string_view>

namespace Thoth::Http {
    template<MethodConcept Method>
    expected<Response<Method>, string> Client::Send(Request<Method> request) {
        const auto endpointOpt{ Hermes::IpEndpoint::TryResolve(request.url.host, to_string(request.url.port)) };
        if (!endpointOpt)
            return std::unexpected{ "DNS Resolution Failed" };

        ClientJanitor& janitor{ ClientJanitor::Instance() };
        std::shared_ptr<Socket> infoPtr;


        {
            std::lock_guard lock(janitor.poolMutex);

            const decltype(janitor.connectionPool)::iterator connContainerIt{ janitor.connectionPool.find(*endpointOpt) };
            if (connContainerIt != janitor.connectionPool.end()) {

                infoPtr = std::move(connContainerIt->second.back());
                connContainerIt->second.pop_back();
            }
        }



        if (!infoPtr) {
            auto newSocketResult{ Hermes::RawTlsClient::Connect(Hermes::TlsSocketData{ *endpointOpt, request.url.host }) };
            if (!newSocketResult)
                return std::unexpected{ "Connection Failed" };

            infoPtr = std::make_shared<Socket>(std::move(*newSocketResult));
        }


        // TODO: implement with rvalue
        request.headers.Add("host", request.url.host);
        request.headers.Add("content-length", to_string(request.body.size()));

        string_view path{ request.url.path.empty() ? string_view{ "/" } : request.url.path };


        const auto requestStr{ std::format(
            "{} {} {}\r\n"
            "{}\r\n"
            "\r\n"
            "{}",
            Method::MethodName(), path, VersionToString(request.version),
            request.headers,
            request.body
        ) };

        const auto [_, sendRes]{ infoPtr->socket.Send(requestStr) };

        if (!sendRes)
            return std::unexpected{ "Send Failed" };


        // TODO: FUTURE: Implement with HTTP2 and 3

        namespace rg = std::ranges;
        namespace vs = std::views;

        Version version{};
        StatusCodeEnum status{};
        string statusMessage{};
        Headers headers{};
        string body{};

        auto stream{ infoPtr->socket.RecvRange<char>() };

        if (!rg::starts_with(stream, string_view{ "HTTP/1." }))
            return std::unexpected{ "Invalid response" };

        switch (*stream.begin()) {
            case '0': version = Version::HTTP1_0; break;
            case '1': version = Version::HTTP1_1; break;
            default: return std::unexpected{ "Invalid version" };
        }
        ++stream.begin();

        const auto arr{ Hermes::Utils::CopyTo<array<char, 5>>(stream) };

        if (arr[0] != ' ' || !isdigit(arr[1]) || !isdigit(arr[2]) || !isdigit(arr[3]) || arr[4] != ' ')
            return std::unexpected{ "Invalid response" };

        status = static_cast<StatusCodeEnum>((arr[1] - '0') * 100 + (arr[2] - '0') * 10 + (arr[3] - '0'));
        statusMessage = stream | Hermes::Utils::UntilMatch(string_view{ "\r\n" }) | rg::to<string>();

        auto rawHeaders{ stream | Hermes::Utils::UntilMatch(string_view{ "\r\n\r\n" }) };
        const auto headersParseRes{ Headers::Parse(rawHeaders) };

        if (!headersParseRes)
            return std::unexpected{ "Invalid headers" };
        headers = *headersParseRes;

        if (auto contentSizeOpt{ headers.Get("content-length") }; contentSizeOpt) {
            const auto temp{ *contentSizeOpt };
            size_t contentSize;

            auto [_, ec] = std::from_chars(temp->data(), temp->data() + temp->size(), contentSize);

            if (ec != std::errc())
                return std::unexpected{ "Invalid response" };

            body = stream | vs::take(contentSize) | rg::to<string>();
        } else {
            if (version == Version::HTTP1_0)
                return std::unexpected{ "HTTP/1.0 needs content-length" };

            static string defaultValue{ "chunked" };
            auto transferEncoding{ *headers.Get("transfer-encoding")
                    .value_or(&defaultValue) };

            if (transferEncoding == "chunked") {
                string chunkLengthStr;
                int chunkLength;
                while (true) {
                    chunkLengthStr = stream | Hermes::Utils::UntilMatch(string_view{ "\r\n" }) | rg::to<string>();
                    auto [_, ec] {
                        std::from_chars( chunkLengthStr.data(), chunkLengthStr.data() + chunkLengthStr.size(),
                    chunkLength, 16)
                    };

                    if (ec != std::errc())
                        return std::unexpected{ "Invalid response" };


                    body.reserve(body.size() + chunkLength);
                    body.append_range(stream | vs::take(chunkLength));

                    if (!rg::starts_with(stream, string_view{ "\r\n" }))
                        return std::unexpected{ "Invalid response" };

                    if (chunkLength == 0)
                        break;
                }
            }
        }

        if (!stream.Error())
                return std::unexpected{ "Something goes wrong when sending" };

        std::lock_guard lock(janitor.poolMutex);
        janitor.connectionPool[*endpointOpt].push_back(std::move(infoPtr));

        return Response<Method>{ version, status, std::move(statusMessage),
            std::move(headers), std::move(body) };
    }
}

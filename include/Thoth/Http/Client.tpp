#pragma once
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/RequestError.hpp>
#include <Thoth/Utils/Overloads.hpp>
#include <Hermes/Utils/UntilMatch.hpp>
#include <string_view>
#include <bit>
#pragma warning(disable: 4455)

namespace Thoth::Http {
    template<MethodConcept Method, BodyConcept Body>
        requires std::default_initializable<Body>
    expected<Response<Method, Body>, RequestError> Client::Send(Request<Method, Body> request) {
        return SendAndRecvAsInto<Method, Body, Body>(request, [](const ResponseHead&) -> std::expected<Body, RequestError> { return {}; });
    }

    template<MethodConcept Method, BodyConcept Body, class F>
        requires ResponseBodyFactoryConcept<F, Body>
    expected<Response<Method, Body>, RequestError> Client::SendAndRecvInto(Request<Method, Body> request, F&& bodyFactory) {
        return SendAndRecvAsInto<Method, Body, Body>(request, std::forward<F>(bodyFactory));
    }


    template<MethodConcept Method, RequestBodyConcept RequestBody, ResponseBodyConcept ResponseBody>
        requires std::default_initializable<ResponseBody>
    expected<Response<Method, ResponseBody>, RequestError> Client::SendAndRecvAs(Request<Method, RequestBody> request) {
        return SendAndRecvAsInto<Method, RequestBodyConcept, ResponseBody>(
            request, [](const ResponseHead&) -> std::expected<ResponseBody, RequestError> { return {}; });
    }


    template<MethodConcept Method, RequestBodyConcept RequestBody, ResponseBodyConcept ResponseBody, class F>
        requires ResponseBodyFactoryConcept<F, ResponseBody>
    expected<Response<Method, ResponseBody>, RequestError> Client::SendAndRecvAsInto(
        Request<Method, RequestBody> request, F&& bodyFactory) {
        ClientJanitor& janitor{ ClientJanitor::Instance() };

        const auto s_establishConnection = [&](Hermes::IpEndpoint&& endpoint) {
            const auto s_getSocketFromPool = [&]() -> std::optional<SocketPtr> {
                std::lock_guard lock{ janitor.poolMutex };

                const decltype(janitor.connectionPool)::iterator connContainerIt{ janitor.connectionPool.find(endpoint) };

                if (connContainerIt == janitor.connectionPool.end() || connContainerIt->second.empty())
                    return std::nullopt;

                auto infoPtr{ std::move(connContainerIt->second.back()) };
                connContainerIt->second.pop_back();

                return std::move(infoPtr);
            };

            const auto s_createNewSocket = [&]() -> std::optional<SocketPtr> {
                auto newSocketResult{ Hermes::RawTlsClient::Connect(Hermes::TlsSocketData{ endpoint, request.url.host }) };
                if (!newSocketResult)
                    return std::nullopt;

                return std::make_shared<Socket>(std::move(*newSocketResult));
            };

            const auto s_cleanupSocket = [&](std::pair<SocketPtr, Response<Method, ResponseBody>> val) {
                std::lock_guard lock{ janitor.poolMutex };


                static Headers::HeaderValue closeConnectionVal{ "close" };
                static Headers::HeaderValue keepAliveConnectionVal{ "keep-alive" };

                const auto connectionHeader{
                    *val.second.headers.Get("connection")
                            .value_or(val.second.version == VersionEnum::HTTP1_0
                                ? &closeConnectionVal
                                : &keepAliveConnectionVal)
                };
                // TODO: FUTURE: Implement the keepAliveHeader (timeout) properly

                if (val.first != nullptr&&  connectionHeader != closeConnectionVal)
                    janitor.connectionPool[endpoint].emplace_back(std::move(val.first));

                return std::move(val.second);
            };

            auto infoPtr{ s_getSocketFromPool().or_else(s_createNewSocket) };

            const auto s_isSocketValid = [&]() -> Hermes::ConnectionResultOper {
                if (!infoPtr)
                    return std::unexpected{ ConnectionErrorEnum::CONNECTION_FAILED };
                return std::monostate{};
            };

            const auto s_sendRequest = [&](std::monostate) -> Hermes::ConnectionResult<SocketPtr> {
                request.headers.Add("host", request.url.host);

                if constexpr (SizedRequestBodyConcept<RequestBody>)
                    request.headers.ContentLength().Set(std::ranges::size(request.body));
                else
                    request.headers.TransferEncoding().Add(NHeaders::TransferEncodingEnum::Chunked);

                const string_view path{ request.url.path.empty() ? string_view{ "/" } : request.url.path };
                const auto& query { request.url.query };
                const auto versionStr{ VersionToString(request.version) };

                // TODO: implement "transfer-encoding: chunked" properly.
                // TODO: change to views::concat later and then create a local buffer to write
                const auto requestStr{ std::format(
                    "{} {}?{} {}\r\n"
                    "{}\r\n"
                    "\r\n"
                    "{}",
                    Method::MethodName(), path, query, versionStr,
                    request.headers,
                    request.body
                ) };

                const auto [_, sendRes]{ (*infoPtr)->socket.Send(requestStr) };

                return sendRes
                        .transform([&](auto){ return std::move(*infoPtr); });
            };

            const auto s_toRequestError = [](const auto err) -> RequestError {
                return RequestError{ err };
            };


            return s_isSocketValid()
                    .and_then(s_sendRequest)
                    .transform_error(s_toRequestError)
                    .and_then(std::bind_back(P_ParseHttp11<Method, ResponseBody, F>, std::forward<F>(bodyFactory)))
                    .transform(s_cleanupSocket);
        };

        const auto s_toRequestError = [](const auto err) {
            return RequestError{ err     };
        };


        return Hermes::IpEndpoint::TryResolve(request.url.host, to_string(request.url.port))
                    .transform_error(s_toRequestError)
                    .and_then(s_establishConnection);
    }

    constexpr auto Client::H_Send() {
        return []<MethodConcept Method, BodyConcept Body>(Request<Method, Body> request) {
            return Send<Method, Body>(request);
        };
    }


    template<class F>
    auto Client::H_SendAndRecvInto(F&& bodyFactory) {
        return [&]<MethodConcept Method, BodyConcept Body>(Request<Method, Body> request) {
            return SendAndRecvInto<Method, Body>(request, std::forward<F>(bodyFactory));
        };
    }

    template<ResponseBodyConcept ResponseBody>
    constexpr auto Client::H_SendAndRecvAs() {
        return []<MethodConcept Method, RequestBodyConcept RequestBody>(Request<Method, RequestBody> request) {
            return SendAndRecvAs<Method, RequestBody, ResponseBody>(request);
        };
    }

    template<ResponseBodyConcept ResponseBody, class F>
        requires ResponseBodyFactoryConcept<F, ResponseBody>
    auto Client::H_SendAndRecvAsInto(F&& bodyFactory) {
        return [&]<MethodConcept Method, RequestBodyConcept RequestBody>(Request<Method, RequestBody> request) {
            return SendAndRecvAsInto<Method, RequestBody, ResponseBody>(request, std::forward<F>(bodyFactory));
        };
    }


    template<MethodConcept Method, ResponseBodyConcept ResponseBody, class F>
        requires ResponseBodyFactoryConcept<F, ResponseBody>
    expected<std::pair<Client::SocketPtr, Response<Method, ResponseBody>>, RequestError> Client::P_ParseHttp11(
        SocketPtr infoPtr, F&& bodyFactory) {
        namespace rg = std::ranges;
        namespace vs = std::views;

        using namespace std::literals;
        using ValueType = ResponseBody::value_type;

        struct ParseHeadStage {
            ResponseHead data;
            decltype(infoPtr->socket.RecvRange<char>()) stream;
            // I could receive based in ValueType, but it would be painful to parse the response head.
            // HTTP/2 will use RecvRange<std::byte>().
        };

        struct ParseCompleteStage : ParseHeadStage {
            ResponseBody body;
        };

        using ParseHeadResult     = std::expected<ParseHeadStage, RequestError>;
        using ParseCompleteResult = std::expected<ParseCompleteStage, RequestError>;


        static constexpr auto s_cvt = [](const char c) { // convert
            return std::bit_cast<ValueType>(c);
        };


        const auto s_createResponseStream = [&]() -> ParseHeadResult {
            return ParseHeadStage{ ResponseHead{}, infoPtr->socket.RecvRange<char>() };
        };

        const auto s_fillResponseLine = [&](ParseHeadStage&& info) -> ParseHeadResult {
            if (!rg::starts_with(info.stream, "HTTP/1."sv))
                return std::unexpected{ RequestError{ RequestBuildErrorEnum::InvalidResponse } };

            switch (*info.stream.begin()) {
                case '0': info.data.version = VersionEnum::HTTP1_0; break;
                case '1': info.data.version = VersionEnum::HTTP1_1; break;
                default: return std::unexpected{ RequestError{ RequestBuildErrorEnum::InvalidVersion } };
            }
            ++info.stream.begin();

            const auto arr{ Hermes::Utils::CopyTo<array<char, 5>>(info.stream) };

            if (arr[0] != ' ' || !isdigit(arr[1]) || !isdigit(arr[2]) || !isdigit(arr[3]) || arr[4] != ' ')
                return std::unexpected{ RequestError{ RequestBuildErrorEnum::InvalidResponse } };

            info.data.status = static_cast<StatusCodeEnum>((arr[1] - '0') * 100 + (arr[2] - '0') * 10 + (arr[3] - '0'));
            info.data.statusMessage = info.stream | Hermes::Utils::UntilMatch("\r\n"sv) | rg::to<string>();

            return std::move(info);
        };

        const auto s_fillHeaders = [&](ParseHeadStage&& info) -> ParseHeadResult {
            auto rawHeaders{ info.stream | Hermes::Utils::UntilMatch("\r\n\r\n"sv) };
            const auto headersParseRes{ Headers::Parse(rawHeaders) };

            if (!headersParseRes)
                return std::unexpected{ RequestError{ RequestBuildErrorEnum::InvalidHeaders } };
            info.data.headers = *headersParseRes;
            return std::move(info);
        };

        const auto s_initializeBody = [&](ParseHeadStage&& info) -> ParseCompleteResult {
            return std::invoke(bodyFactory, info.data)
                    .transform([info = std::move(info)](auto&& body) {
                        return ParseCompleteStage{
                            std::move(info.data),
                            std::move(info.stream),
                            std::move(body)
                        };
                    });
        };

        const auto s_fillBody = [&](ParseCompleteStage&& info) -> ParseCompleteResult {
            using State1 = std::expected<std::variant<std::monostate, size_t>, NHeaders::HeaderErrorEnum>;
            using State2 = std::expected<std::variant<std::monostate, size_t>, RequestError>;

            const auto s_extractChunked = [](vector<NHeaders::TransferEncodingEnum> values) -> State1 {
                if (std::ranges::contains(values, NHeaders::TransferEncodingEnum::Chunked))
                    return { {std::monostate{} } };

                return unexpected{ NHeaders::HeaderErrorEnum::NotFound };
            };

            const auto s_extractLengthIfNotChunked = [&](NHeaders::HeaderErrorEnum error) -> State2 {
                if (error == NHeaders::HeaderErrorEnum::NotFound)
                    if (const auto res{ info.data.headers.ContentLength().GetWithDefault(0) }; res)
                        return *res;

                return std::unexpected{ RequestError{ RequestBuildErrorEnum::InvalidHeaders } };
            };

            const auto s_readBody = [&](State2::value_type value) -> ParseCompleteResult {
                const auto s_readSizedLength = [&](const size_t contentSize) -> std::expected<std::monostate, RequestError> {
                    if constexpr (requires (ResponseBody b){ { b.resize(0) }; })
                        info.body.resize(contentSize);

                    rg::copy(
                        info.stream
                                | vs::take(contentSize)
                                | vs::transform(s_cvt),
                        GetInserterIterator(info.body)
                    );

                    return std::monostate{};
                };

                const auto s_readChunked = [&](std::monostate) -> std::expected<std::monostate, RequestError> {
                    if (info.data.version == VersionEnum::HTTP1_0)
                        return std::unexpected{ RequestError{ RequestBuildErrorEnum::VersionNeedsContentLength } };

                    string chunkLengthStr;
                    decltype(NHeaders::Scan<size_t>(chunkLengthStr)) chunkLength;


                    do {
                        chunkLengthStr.clear();
                        rg::copy(info.stream | Hermes::Utils::UntilMatch("\r\n"sv), std::back_inserter(chunkLengthStr));
                        chunkLength = NHeaders::Scan<size_t>(chunkLengthStr, 16);

                        if (!chunkLength)
                            return std::unexpected{ RequestError{ RequestBuildErrorEnum::InvalidResponse } };

                        rg::copy(
                            info.stream
                                    | vs::take(*chunkLength)
                                    | vs::transform(s_cvt),
                            GetInserterIterator(info.body)
                        );

                        //? Support extensions too? Maybe, someday
                        if (!rg::starts_with(info.stream, "\r\n"sv))
                            return std::unexpected{ RequestError{ RequestBuildErrorEnum::InvalidResponse } };

                    } while (chunkLength != 0);

                    return std::monostate{};
                };

                return std::visit(Utils::Overloaded{ s_readSizedLength, s_readChunked }, value)
                        .transform([&](auto){ return std::move(info); });
            };


            return info.data.headers.TransferEncoding().Get()
                    .and_then(s_extractChunked)
                    .or_else(s_extractLengthIfNotChunked)
                    .and_then(s_readBody);
        };

        const auto s_createObject = [&](ParseCompleteStage&& info) {
            return std::pair{
                    std::move(infoPtr),
                    Response<Method, ResponseBody>{
                        info.data.version, info.data.status, std::move(info.data.statusMessage),
                        std::move(info.data.headers),
                        std::move(info.body)
                    }
                };
        };


        return s_createResponseStream()
                .and_then(s_fillResponseLine)
                .and_then(s_fillHeaders)
                .and_then(s_initializeBody)
                .and_then(s_fillBody)
                .transform(s_createObject);

    }
}

#pragma warning(default: 4455)
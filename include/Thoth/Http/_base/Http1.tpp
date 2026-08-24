#pragma once
#pragma region Macros
#pragma push_macro("ASSERT_OR_RET_ERROR")
#pragma push_macro("SEND_OR_RET_ERROR")
#pragma push_macro("HTTP11_FORWARD")
#pragma push_macro("VALID_STREAM")
#undef ASSERT_OR_RET_ERROR
#undef SEND_OR_RET_ERROR
#undef HTTP11_FORWARD
#undef VALID_STREAM

#define ASSERT_OR_RET_ERROR(cond, error) do { \
    if (!(cond)) return ThothUnex{ (error) }; \
} while (0)

#define SEND_OR_RET_ERROR(varName, input) do {                           \
    const auto [m_ ## varName, varName]{ socket.Send(input, options) };  \
    ASSERT_OR_RET_ERROR(varName, varName.error());                       \
} while (0)

#define HTTP11_FORWARD(methodName) ([](auto stage) { return Http1::methodName(std::move(stage)); })

#define VALID_STREAM(stream) do {                    \
    if constexpr (requires { (stream).Error(); }) {  \
        const auto streamError{ (stream).Error() };  \
        if (!streamError)                            \
            return ThothUnex{ streamError.error() }; \
    }                                                \
} while (0)

#pragma endregion

namespace Thoth::Http::details_ {

    template<class Method, WritableBodyConcept ResponseBody, class F, class Stream>
        requires ResponseBodyFactoryConcept<F, ResponseBody>
    std::expected<Response<Method, ResponseBody>, ThothError> Http1::BuildResponse(
        Stream&& stream, F&& bodyFactory, std::size_t maxBodyLength) {
        using HeadStage     = ResponseParseStage<Stream>;
        using CompleteStage = ResponseParseCompleteStage<Stream, ResponseBody>;

        const auto createResponseStream{ [&](auto&& s) -> std::expected<HeadStage, ThothError> {
            return HeadStage{ ResponseHead{}, s };
        } };

        const auto initializeBody{ [&](HeadStage stage) -> std::expected<CompleteStage, ThothError> {
            auto bodyExp{ std::invoke(bodyFactory, stage.data) };
            ASSERT_OR_RET_ERROR(bodyExp, bodyExp.error());

            return CompleteStage{ { std::move(stage.data), std::move(stage.stream) }, std::move(*bodyExp) };
        } };

        const auto parseBody{ [maxBodyLength](CompleteStage stage) {
            return Http1::ParseBody(std::move(stage), maxBodyLength);
        } };

        const auto createResponse{ [](CompleteStage&& stage) {
            return Response<Method, ResponseBody>{ stage.data, std::move(stage.body) };
        } };

        return createResponseStream(std::forward<Stream>(stream))
                .and_then(HTTP11_FORWARD(ParseResponseLine))
                .and_then(HTTP11_FORWARD(ParseHeaders))
                .and_then(initializeBody)
                .and_then(parseBody)
                .transform(createResponse);
    }

    template<class Stream>
    std::expected<ResponseParseStage<Stream>, ThothError> Http1::ParseResponseLine(ResponseParseStage<Stream> stage) {
        namespace rg = std::ranges;
        namespace vs = std::views;
        using namespace std::literals;

        static constexpr auto k_maxStatusMessageSize{ 1024 };

        ASSERT_OR_RET_ERROR(rg::starts_with(stage.stream, "HTTP/1."sv), MessageParseErrorEnum::InvalidStartLine);

        switch (*stage.stream.begin()) {
            case '0': stage.data.version = VersionEnum::HTTP1_0; break;
            case '1': stage.data.version = VersionEnum::HTTP1_1; break;
            default: return ThothUnex{ MessageParseErrorEnum::InvalidVersion };
        }
        ++stage.stream.begin();

        const auto arr{ Hermes::Utils::ExtractTo<std::array<char, 5>>(stage.stream) };

        ASSERT_OR_RET_ERROR(arr[0] == ' ' && isdigit(arr[1]) && isdigit(arr[2]) && isdigit(arr[3]) && arr[4] == ' ',
                            MessageParseErrorEnum::InvalidStartLine);

        stage.data.status = static_cast<StatusCodeEnum>((arr[1] - '0') * 100 + (arr[2] - '0') * 10 + (arr[3] - '0'));
        stage.data.statusMessage = stage.stream
                | vs::take(k_maxStatusMessageSize + k_crlf.size())
                | Hermes::Utils::UntilMatch<true>(k_crlf)
                | rg::to<std::string>();

        if (!stage.data.statusMessage.ends_with(k_crlf))
            return std::unexpected{ MessageParseErrorEnum::InvalidStartLine };

        for (auto _ : k_crlf) stage.data.statusMessage.pop_back();

        VALID_STREAM(stage.stream);

        return std::move(stage);
    }

    inline std::expected<std::monostate, ThothError> Http1::ValidateFraming(const Headers& headers) {
        if (headers.Exists("content-length") && headers.Exists("transfer-encoding"))
            return ThothUnex{ MessageParseErrorEnum::InvalidHeaders };;

        if (headers.Exists("transfer-encoding")) {
            const auto transferEncoding{ headers.TransferEncoding().Get() };
            if (!transferEncoding || transferEncoding->size() != 1
                || transferEncoding->front() != NHeaders::TransferEncodingEnum::Chunked)
                return ThothUnex{ MessageParseErrorEnum::InvalidHeaders };;
        }

        return std::monostate{};
    }

    template<class Stream, class Head>
        std::expected<ParseStage<Stream, Head>, ThothError> Http1::ParseHeaders(ParseStage<Stream, Head> stage) {
        using namespace std::literals;
        using HeadersType = decltype(stage.data.headers);

        auto rawHeaders{ stage.stream | Hermes::Utils::UntilMatch(k_crlfCrlf) };
        const auto headersParseRes{ HeadersType::Parse(rawHeaders) };
        VALID_STREAM(stage.stream);

        // headersParseRes.error() is a StatusCodeEnum (BadRequest / ContentTooLarge) - requires
        // Http::StatusCodeEnum to be a ThothErrorBase alternative (see ThothError.hpp) to compile.
        ASSERT_OR_RET_ERROR(headersParseRes, headersParseRes.error());
        ASSERT_OR_RET_ERROR(ValidateFraming(*headersParseRes), MessageParseErrorEnum::InvalidHeaders);

        stage.data.headers = std::move(*headersParseRes);

        return std::move(stage);
    }




    template<ReadableBodyConcept Body>
    void Http1::PrepareBodyHeaders(Headers& headers, const Body& body) {
        headers.Remove("transfer-encoding");
        headers.Remove("content-length");

        if constexpr (SizedReadableBodyConcept<Body>) {
            headers.ContentLength().Set(std::ranges::size(body));
        } else {
            headers.TransferEncoding().Add(NHeaders::TransferEncodingEnum::Chunked);
        }
    }

template<class Stream, WritableBodyConcept Body, class Head>
    std::expected<ParseCompleteStage<Stream, Head, Body>, ThothError> Http1::ParseBody(
        ParseCompleteStage<Stream, Head, Body> stage, std::size_t maxBodyLength)
    {
        namespace rg = std::ranges;
        namespace vs = std::views;
        using namespace std::literals;
        using ValueType = typename Body::value_type;

        using ParseErrEnum         = MessageParseErrorEnum;
        using HeaderErrEnum        = NHeaders::HeaderErrorEnum;
        using TransEncodingErrEnum = NHeaders::TransferEncodingEnum;

        static constexpr auto cvt{ [](const char c) {
            return std::bit_cast<ValueType>(c);
        } };

        using TransferValue = std::variant<std::monostate, size_t>;
        using State1 = std::expected<TransferValue, HeaderErrEnum>;
        using State2 = std::expected<TransferValue, ThothError>;

        const auto extractChunked{ [](const std::vector<TransEncodingErrEnum>& values) -> State1 {
            if (rg::contains(values, TransEncodingErrEnum::Chunked))
                return TransferValue{ std::monostate{} };
            return std::unexpected{ HeaderErrEnum::NotFound };
        } };

        const auto extractLengthIfNotChunked{ [&](HeaderErrEnum error) -> State2 {
            if (error == HeaderErrEnum::NotFound)
                if (const auto res{ stage.data.headers.ContentLength().GetWithDefault(0) }; res)
                    return TransferValue{*res};

            return ThothUnex{ ParseErrEnum::InvalidHeaders };
        } };

        const auto readBody{ [&](State2::value_type value) -> std::expected<std::monostate, ThothError> {
            const auto readSizedLength{ [&](const size_t contentSize) -> std::expected<std::monostate, ThothError> {
                if (contentSize > maxBodyLength)
                    return ThothUnex{ ParseErrEnum::InvalidHeaders };

                if constexpr (requires (Body b){ { b.reserve(0) }; })
                    stage.body.reserve(contentSize);

                rg::copy(
                    stage.stream | vs::take(contentSize) | vs::transform(cvt),
                    GetInserterIterator(stage.body)
                );

                VALID_STREAM(stage.stream);
                return std::monostate{};
            } };

            const auto readChunked{ [&](std::monostate) -> std::expected<std::monostate, ThothError> {
                ASSERT_OR_RET_ERROR(stage.data.version != VersionEnum::HTTP1_0, ParseErrEnum::VersionNeedsContentLength);

                // Big enough so that the parser will return a error if the pattern wasn't found.
                static constexpr auto k_maxChunkLineLength{ 64 };

                std::string chunkLengthStr;
                decltype(Utils::Scan<size_t>(chunkLengthStr)) chunkLength;
                size_t totalBodySize{};

                do {
                    chunkLengthStr.clear();
                    rg::copy(
                        stage.stream
                            | vs::take(k_maxChunkLineLength + k_crlf.size())
                            | Hermes::Utils::UntilMatch<true>(k_crlf),
                        std::back_inserter(chunkLengthStr)
                    );
                    VALID_STREAM(stage.stream);

                    ASSERT_OR_RET_ERROR(chunkLengthStr.ends_with(k_crlf), ParseErrEnum::InvalidStartLine);
                    for (auto _ : k_crlf) chunkLengthStr.pop_back();

                    chunkLength = Utils::Scan<size_t>(chunkLengthStr, "x");
                    ASSERT_OR_RET_ERROR(chunkLength, ParseErrEnum::InvalidStartLine);

                    ASSERT_OR_RET_ERROR(totalBodySize + *chunkLength <= maxBodyLength, ParseErrEnum::InvalidHeaders);
                    totalBodySize += *chunkLength;

                    rg::copy(
                        stage.stream | vs::take(*chunkLength) | vs::transform(cvt),
                        GetInserterIterator(stage.body)
                    );
                    VALID_STREAM(stage.stream);

                    ASSERT_OR_RET_ERROR(rg::starts_with(stage.stream, k_crlf), ParseErrEnum::InvalidStartLine);
                } while (chunkLength != 0);

                return std::monostate{};
            } };

            return std::visit(Hermes::Utils::Overloaded{ readSizedLength, readChunked }, value);
        } };

        auto readRes{ stage.data.headers.TransferEncoding().Get()
                .and_then(extractChunked)
                .or_else(extractLengthIfNotChunked)
                .and_then(readBody) };

        if (!readRes) return std::unexpected{ readRes.error() };

        return std::move(stage);
    }

    template<MethodConcept Method, class Head, ConnectionConcept Socket>
        requires (std::same_as<Head, RequestHead> || std::same_as<Head, ResponseHead>)
    std::expected<std::monostate, ThothError> Http1::SendMessageHead(
        Socket& socket, const Head& head, typename Socket::SendOptions options) {
        std::string requestStr{ std::format("{} {}", Method::MethodName(), head) };
        SEND_OR_RET_ERROR(res, requestStr);

        return std::monostate{};
    }

    template<ConnectionConcept Socket, ReadableBodyConcept Body>
    std::expected<size_t, ThothError> Http1::SendBody(
        Socket& socket, const Body& body, typename Socket::SendOptions options) {
        namespace rg = std::ranges;
        size_t totalBytes{};

        if constexpr (SizedReadableBodyConcept<Body>) {
            std::string_view data{ reinterpret_cast<const char*>(std::ranges::data(body)), std::ranges::size(body) };

            SEND_OR_RET_ERROR(res, data);
            totalBytes += data.size();
        } else {
            for (const auto& chunk : body) {
                std::string_view chunkData{ reinterpret_cast<const char*>(rg::data(chunk)), rg::size(chunk) };
                if (chunkData.empty()) continue;

                std::string header{ std::format("{:x}{}", chunkData.size(), k_crlf) };

                SEND_OR_RET_ERROR(hRes, header);
                SEND_OR_RET_ERROR(dRes, chunkData);
                SEND_OR_RET_ERROR(cRes, k_crlf);

                totalBytes += chunkData.size();
            }

            SEND_OR_RET_ERROR(endRes, k_lastChunk);
        }

        return totalBytes;
    }
}

#pragma pop_macro("VALID_STREAM")
#pragma pop_macro("HTTP11_FORWARD")
#pragma pop_macro("SEND_OR_RET_ERROR")
#pragma pop_macro("ASSERT_OR_RET_ERROR")
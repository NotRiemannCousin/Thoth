#pragma once
#pragma region Macros
#pragma push_macro("ASSERT_OR_RET_ERROR")
#pragma push_macro("SEND_OR_RET_ERROR")
#undef ASSERT_OR_RET_ERROR
#undef SEND_OR_RET_ERROR

#define ASSERT_OR_RET_ERROR(cond, error) do {                        \
    if (!(cond)) return std::unexpected{ ExchangeError{ (error) } }; \
} while (0)

#define SEND_OR_RET_ERROR(varName, input) do {                     \
    const auto [_ ## varName, varName]{ socket.Send(input) };      \
    ASSERT_OR_RET_ERROR(varName, varName.error());                 \
} while (0)
#pragma endregion

namespace Thoth::Http::details_ {
    template <class Stream>
    std::expected<ResponseParseStage<Stream>, ExchangeError> Http11::ParseResponseLine(ResponseParseStage<Stream> stage) {
        namespace rg = std::ranges;
        using namespace std::literals;

        ASSERT_OR_RET_ERROR(rg::starts_with(stage.stream, "HTTP/1."sv), MessageParseErrorEnum::InvalidStartLine);

        switch (*stage.stream.begin()) {
            case '0': stage.data.version = VersionEnum::HTTP1_0; break;
            case '1': stage.data.version = VersionEnum::HTTP1_1; break;
            default: return std::unexpected{ ExchangeError{ MessageParseErrorEnum::InvalidVersion } };
        }
        ++stage.stream.begin();

        const auto arr{ Hermes::Utils::ExtractTo<std::array<char, 5>>(stage.stream) };

        ASSERT_OR_RET_ERROR(arr[0] == ' ' && isdigit(arr[1]) && isdigit(arr[2]) && isdigit(arr[3]) && arr[4] == ' ',
                            MessageParseErrorEnum::InvalidStartLine);

        stage.data.status = static_cast<StatusCodeEnum>((arr[1] - '0') * 100 + (arr[2] - '0') * 10 + (arr[3] - '0'));
        stage.data.statusMessage = stage.stream | Hermes::Utils::UntilMatch(k_crlf) | rg::to<std::string>();

        return std::move(stage);
    }

    template<class Stream, class Head>
        std::expected<ParseStage<Stream, Head>, ExchangeError> Http11::ParseHeaders(ParseStage<Stream, Head> stage) {
        using namespace std::literals;
        using HeadersType = decltype(stage.data.headers);

        auto rawHeaders{ stage.stream | Hermes::Utils::UntilMatch(k_crlfCrlf) };
        const auto headersParseRes{ HeadersType::Parse(rawHeaders) };

        ASSERT_OR_RET_ERROR(headersParseRes, MessageParseErrorEnum::InvalidHeaders);

        stage.data.headers = std::move(*headersParseRes);

        return std::move(stage);
    }






    template<ReadableBodyConcept Body>
    void Http11::PrepareBodyHeaders(Headers& headers, const Body& body) {
        if constexpr (SizedReadableBodyConcept<Body>) {
            headers.ContentLength().Set(std::ranges::size(body));
        } else {
            headers.TransferEncoding().Add(NHeaders::TransferEncodingEnum::Chunked);
        }
    }

template<class Stream, WritableBodyConcept Body, class Head>
    std::expected<ParseCompleteStage<Stream, Head, Body>, ExchangeError> Http11::ParseBody(
        ParseCompleteStage<Stream, Head, Body> stage)
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
        using State2 = std::expected<TransferValue, ExchangeError>;

        const auto extractChunked{ [](const std::vector<TransEncodingErrEnum>& values) -> State1 {
            if (rg::contains(values, TransEncodingErrEnum::Chunked))
                return TransferValue{ std::monostate{} };
            return std::unexpected{ HeaderErrEnum::NotFound };
        } };

        const auto extractLengthIfNotChunked{ [&](HeaderErrEnum error) -> State2 {
            if (error == HeaderErrEnum::NotFound)
                if (const auto res{ stage.data.headers.ContentLength().GetWithDefault(0) }; res)
                    return TransferValue{*res};

            return std::unexpected{ ExchangeError{ ParseErrEnum::InvalidHeaders } };
        } };

        const auto readBody{ [&](State2::value_type value) -> std::expected<std::monostate, ExchangeError> {
            const auto readSizedLength{ [&](const size_t contentSize) -> std::expected<std::monostate, ExchangeError> {
                if constexpr (requires (Body b){ { b.reserve(0) }; })
                    stage.body.reserve(contentSize);

                rg::copy(
                    stage.stream | vs::take(contentSize) | vs::transform(cvt),
                    GetInserterIterator(stage.body)
                );
                return std::monostate{};
            } };

            const auto readChunked{ [&](std::monostate) -> std::expected<std::monostate, ExchangeError> {
                ASSERT_OR_RET_ERROR(stage.data.version != VersionEnum::HTTP1_0, ParseErrEnum::VersionNeedsContentLength);

                std::string chunkLengthStr;
                decltype(Utils::Scan<size_t>(chunkLengthStr)) chunkLength;

                do {
                    chunkLengthStr.clear();
                    rg::copy(stage.stream | Hermes::Utils::UntilMatch(k_crlf), std::back_inserter(chunkLengthStr));
                    chunkLength = Utils::Scan<size_t>(chunkLengthStr, "x");

                    ASSERT_OR_RET_ERROR(chunkLength, ParseErrEnum::InvalidStartLine);

                    rg::copy(
                        stage.stream | vs::take(*chunkLength) | vs::transform(cvt),
                        GetInserterIterator(stage.body)
                    );

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

    template<MethodConcept Method, class Head, WireSocketConcept Socket>
        requires (std::same_as<Head, RequestHead> || std::same_as<Head, ResponseHead>)
    std::expected<std::monostate, ExchangeError> Http11::SendMessageHead(Socket& socket, const Head& head) {
        std::string requestStr{ std::format("{} {}", Method::MethodName(), head) };
        SEND_OR_RET_ERROR(res, requestStr);

        return std::monostate{};
    }

    template<WireSocketConcept Socket, ReadableBodyConcept Body>
    std::expected<size_t, ExchangeError> Http11::SendBody(Socket& socket, const Body& body) {
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

#pragma pop_macro("SEND_OR_RET_ERROR")
#pragma pop_macro("ASSERT_OR_RET_ERROR")
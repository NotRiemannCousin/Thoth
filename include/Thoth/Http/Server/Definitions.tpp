#pragma once

namespace Thoth::Http {
    template<CommonSocketConcept Socket>
    template<IncomingBodyConcept Body, class F>
        requires RequestBodyFactoryConcept<F, Body>
    exec::task<Body> Exchange<Socket>::ReadAs(F&& factory) {
        bodyRead = true;
        co_return co_await Transport::Http1::ReadBody<Socket, Body>(context, head, std::forward<F>(factory));
    }

    template<CommonSocketConcept Socket>
    template<MethodConcept Method, IncomingBodyConcept Body>
    exec::task<Request<Method, Body>> Exchange<Socket>::ReadRequest() {
        auto body{ co_await ReadAs<Body>([](const RequestHead&) -> std::expected<Body, ThothError> {
            return Body{};
        }) };
        co_return Request<Method, Body>{ head, std::move(body) };
    }

    template<CommonSocketConcept Socket>
    bool Exchange<Socket>::HasBody() const noexcept {
        return std::visit(Hermes::Utils::Overloaded{
            [](FramingEmpty   ) { return false; },
            [](FramingSized  s) { return s.size != 0; },
            [](FramingStreamed) { return true; },
        }, *this);
    }

    template<CommonSocketConcept Socket>
    bool Exchange<Socket>::Streamed() const noexcept {
        return std::holds_alternative<FramingStreamed>(*this);
    }

    template<CommonSocketConcept Socket>
    exec::task<Exchange<Socket>> ReceiveHead(ReadContext<Socket>& context, const HostPolicy& hosts) {
        auto headRes{ co_await Transport::Http1::ReadRequestHead(context) };
        RequestHead& head{ headRes.second };

        BodyFraming framing{};
        if (head.headers.TransferEncoding().IsSetted())
            framing = FramingStreamed{};
        else if (const auto size{ head.headers.ContentLength().GetWithDefault(0) }) {
            framing = FramingSized{ *size };

            if (*size > context.limits.maxBodyBytes)
                throw ThothError{ StatusCodeEnum::ContentTooLarge };
        }

        if (const auto hostUrl{ head.headers.Host().GetAsOpt() }; hostUrl && !hostUrl->empty()) {
            const auto hostAllowed{ [&hosts](std::string_view host) {
                const auto stripPort{ [](std::string_view value) {
                    if (const auto colon{ value.rfind(':') }; colon != std::string_view::npos)
                        return value.substr(0, colon);
                    return value;
                } };

                if (!hosts.matchPort)
                    host = stripPort(host);

                for (const auto& allowed : hosts.allowed) {
                    std::string_view candidate{ allowed };
                    if (!hosts.matchPort)
                        candidate = stripPort(candidate);

                    if (String::TokenEquals(host, candidate))
                        return true;
                }

                return false;
            } };

            if (!hostAllowed(*hostUrl))
                throw ThothError{ StatusCodeEnum::BadRequest };
        } else if (!hosts.fallbackHost) {
            throw ThothError{ StatusCodeEnum::BadRequest };
        }

        co_return Exchange<Socket>{
            .method  = std::move(headRes.first),
            .head    = std::move(headRes.second),
            .framing = framing,
            .context = context,
        };
    }
}
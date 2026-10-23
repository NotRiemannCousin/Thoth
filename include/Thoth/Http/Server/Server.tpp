#pragma once

#include <exception>
#include <utility>
#include <variant>

#include <exec/repeat_until.hpp>
#include <exec/finally.hpp>
#include <exec/task.hpp>
#include <stdexec/execution.hpp>

namespace Thoth::Http {

    namespace details_ {
        inline ThothError ExceptionToExpected(const std::exception_ptr& failure) noexcept {
                try { std::rethrow_exception(failure); }
                catch (const NJson::JsonParseError&     e) { return ThothError{ e }; }
                catch (const NJson::JsonGetError&       e) { return ThothError{ e }; }
                catch (const NJson::JsonFindError&      e) { return ThothError{ e }; }
                catch (const NJson::JsonSearchError&    e) { return ThothError{ e }; }
                catch (const NJson::JsonWrongTypeError& e) { return ThothError{ e }; }
                catch (const UrlParseErrorEnum&         e) { return ThothError{ e }; }
                catch (const ConnectionErrorEnum&       e) { return ThothError{ e }; }
                catch (const MessageParseErrorEnum&     e) { return ThothError{ e }; }
                catch (const StatusCodeEnum&            e) { return ThothError{ e }; }
                catch (const GenericError&              e) { return ThothError{ e }; }
                catch (const ThothError&                e) { return ThothError{ e }; }
                catch (const Hermes::TransferError& transfer) { return ThothError{ transfer.error }; }
                catch (...) { return ThothError{ StatusCodeEnum::InternalServerError }; }
            }

        // "At least I tried to reply...".
        template<CommonSocketConcept SocketT>
        auto SendBestEffort(SocketT& conn, const StatusCodeEnum status) {
            ResponseHeaders headers;
            headers.ContentLength().Set(0);
            headers.Connection().Add("close");

            return stdexec::just(ResponseHead{ VersionEnum::Http1_1, status, std::move(headers) })
                    | stdexec::let_value([&conn](const ResponseHead& head) noexcept {
                        return Transport::Http1::WriteResponseHead(conn, head);
                    })
                    | stdexec::upon_error([](auto&&) noexcept {});
        }

        template<CommonSocketConcept SocketT>
        exec::task<void> ReplyAndClose(SocketT& conn, const ThothError& failure) {
            const StatusCodeEnum status{ std::visit(Hermes::Utils::Overloaded{
                   [](const StatusCodeEnum& s   ) { return s; },
                   [](const ConnectionErrorEnum&) { return StatusCodeEnum::InternalServerError; },
                   [](const GenericError&       ) { return StatusCodeEnum::InternalServerError; },
                   [](const auto&               ) { return StatusCodeEnum::BadRequest; }
               }, failure) };

            if (!failure.Is<Hermes::ConnectionErrorEnum>())
                co_await SendBestEffort(conn, status);

            co_return;
        }

        template<CommonSocketConcept SocketT>
        auto DrainUnreadBody(ReadContext<SocketT>& context, const RequestHead& head) {
            return Transport::Http1::ReadBody<SocketT, EmptyBody>(context, head)
                 | stdexec::then([](auto&&) { return true; })
                 | stdexec::upon_error([](auto&&) { return false; });
        }

        //! @brief How many accept rejections in a row a listener survives before the loop stops believing in a
        //! next one. A rejected accept is not a reason to die, but the same error N times in a row with no
        //! success in between is a dead listener spinning on the CPU (an EMFILE that never heals, a closed
        //! socket nobody closed properly), that one gets to die loud instead of looping hot forever.
        inline constexpr std::size_t k_maxAcceptFailures{ 64 };
    }

#pragma region Constructors

    template<class Router, Hermes::AsyncListenerSocketConcept Socket>
    template<class>
    Server<Router, Socket>::Server(Data&& data, const int backlog)
        requires std::default_initializable<ListenOptions> : m_data{ std::move(data) }, m_backlog{ backlog } {}

    template<class Router, Hermes::AsyncListenerSocketConcept Socket>
    Server<Router, Socket>::Server(Data&& data, ListenOptions options, const int backlog) :
                                m_data{ std::move(data) }, m_options{ options }, m_backlog{ backlog } {}

#pragma endregion


#pragma region Router

    template<class Router, Hermes::AsyncListenerSocketConcept Socket>
    Router* Server<Router, Socket>::operator->() noexcept {
        return &m_router;
    }

    template<class Router, Hermes::AsyncListenerSocketConcept Socket>
    const Router* Server<Router, Socket>::operator->() const noexcept {
        return &m_router;
    }

#pragma endregion


#pragma region Run

    template<class Router, Hermes::AsyncListenerSocketConcept Socket>
    auto Server<Router, Socket>::Run() const -> stdexec::sender auto {

        auto serverLoop{ [this](Socket& serverListener) -> stdexec::sender auto {
            m_isRunning.store(true, std::memory_order_release);
            m_acceptFailureStreak = 0;

            if (m_stop.stop_requested())
                serverListener.Close();

            auto acceptOne{ [this, &serverListener] {

                return serverListener.AsyncAcceptOne()
                        | stdexec::then([this](ServerSocket acceptedConn) {
                            if (m_stop.stop_requested()) {
                                acceptedConn.Close();
                                return true;
                            }
                            m_acceptFailureStreak = 0;
                            m_scope.spawn(HandleConnection(std::move(acceptedConn)));
                            return false;
                        })
                        | stdexec::upon_error([this](auto) noexcept {
                            if (m_stop.stop_requested())
                                return true;

                            // TODO: to many failures kills the server, I need to chose something better in the future
                            // TODO: Also, I need to implement logging someday.
                            if (++m_acceptFailureStreak >= details_::k_maxAcceptFailures)
                                return true;

                            return false;
                        });
            } };

            return stdexec::write_env(
                stdexec::just()
                        | stdexec::let_value(std::move(acceptOne))
                        | exec::repeat_until(),
                stdexec::prop{ stdexec::get_stop_token, m_stop.get_token() }
            );
        } };

        auto terminateConnections{ [this] {
            m_scope.request_stop();
            return m_scope.on_empty();
        } };

        auto serverLifecycle{
            Socket::Listen(std::move(m_data), m_options, m_backlog)
                    | stdexec::let_error([](auto err){ return stdexec::just_error(ThothError{ err }); })

                    | stdexec::let_value(std::move(serverLoop))

                    | stdexec::let_value(terminateConnections)
                    | stdexec::let_stopped(terminateConnections)
                    | stdexec::let_error([this](auto err) {
                        return m_scope.on_empty()
                            | stdexec::let_value([err] { return stdexec::just_error(err); });
                    })

                    | stdexec::let_value([]{ return stdexec::just_stopped(); })
        };

        return exec::finally(
            std::move(serverLifecycle),
            stdexec::just() | stdexec::then([this] { m_isRunning.store(false, std::memory_order_release); })
        );
    }

    template<class Router, Hermes::AsyncListenerSocketConcept Socket>
    void Server<Router, Socket>::Stop(bool graceful, std::chrono::milliseconds deadline) const {
        // TODO: implement graceful and deadline
        m_isRunning.store(false, std::memory_order_release);
        m_stop.request_stop();
        m_scope.request_stop();
    }

    template<class Router, Hermes::AsyncListenerSocketConcept Socket>
    bool Server<Router, Socket>::Running() const noexcept {
        return m_isRunning;
    }



    template<class Router, Hermes::AsyncListenerSocketConcept Socket>
    auto Server<Router, Socket>::HandleConnection(ServerSocket conn) const {
        struct ConnectionInfo {
            ServerSocket socket;
            ExchangeBuffer window{};
            ReadContext<ServerSocket> context;
            std::size_t reqCount{};

            ConnectionInfo(ServerSocket&& sock, HttpLimits limits)
                    : socket{ std::move(sock) }, context{ socket, window, limits } {}
        };

        std::shared_ptr info{ std::make_shared<ConnectionInfo>(std::move(conn), limits) };

        auto handle{ [this, info] {
            return stdexec::just()
                    | stdexec::let_value([this, info]{ return HandleOne(info->context, info->socket, info->reqCount); })
                    | stdexec::then([=](bool keepAlive){ ++info->reqCount; return !keepAlive; });
        } };

        return stdexec::just()
                | stdexec::let_value(std::move(handle))
                | exec::repeat_until()
                | stdexec::let_error([info](std::exception_ptr failure) noexcept {
                    return details_::ReplyAndClose(info->socket, details_::ExceptionToExpected(std::move(failure)))
                            | stdexec::upon_error([](auto...){});
                });
    }

    template<class Router, Hermes::AsyncListenerSocketConcept Socket>
    exec::task<bool> Server<Router, Socket>::HandleOne(
        ReadContext<ServerSocket>& context, ServerSocket& conn, const std::size_t requestCount) const {
        Exchange<ServerSocket> exchange{ co_await ReceiveHead(context, hosts) };

        std::pair<ResponseHead, AnyBodyOut> resolved{ co_await m_router.Resolve(exchange) };

        if (const bool keepAlive{ co_await WriteResponse(exchange, std::move(resolved), conn, requestCount) }; !keepAlive)
            co_return false;

        if (!exchange.bodyRead)
            co_return co_await details_::DrainUnreadBody(context, exchange.head);

        co_return true;
    }

    template<class Router, Hermes::AsyncListenerSocketConcept Socket>
    auto Server<Router, Socket>::WriteResponse(const Exchange<ServerSocket>& request, std::pair<ResponseHead, AnyBodyOut> response,
            ServerSocket& conn, const std::size_t requestCount) const noexcept {
        using Transport::Http1; // TODO: I don't know when I will use HTTP3 but im marking this to change
        struct ResponseReply {
            std::pair<ResponseHead, AnyBodyOut> response;
            bool close{};
        };

        auto prepareSend{ [&] mutable {
            const bool close{
                m_stop.stop_requested()
                || requestCount >= limits.maxRequestsPerConnection
                || (!request.bodyRead && std::holds_alternative<FramingStreamed>(request.framing))
                || !Transport::HeadWantsKeepAlive(request.head)
                || !Transport::HeadWantsKeepAlive(response.first)
            };

            if (close) response.first.headers.Connection().TrySet("close");

            Http1::PrepareBodyHeaders(response.first.headers, response.second);

            return ResponseReply{ std::move(response), close };
        } };

        return stdexec::just(prepareSend())
                | stdexec::let_value([&conn](ResponseReply& reply) {
                    return Http1::WriteResponseHead(conn, reply.response.first)
                            | stdexec::let_value([&conn, &reply] { return Http1::WriteBody(conn, reply.response.second); })
                            | stdexec::then([close = reply.close](std::size_t) { return !close; });
                });
    }

#pragma endregion
}

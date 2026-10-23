#pragma once
#include <atomic>
#include <chrono>
#include <concepts>
#include <stop_token>

#include <exec/async_scope.hpp>
#include <exec/task.hpp>

#include <Hermes/Socket/Async/AsyncListenerSocket.hpp>

#include <Thoth/Http/Server/Router.hpp>
#include <Thoth/Http/Server/Definitions.hpp>
#include <Thoth/Http/Server/Routers/BasicRouter.hpp>

namespace Thoth::Http {

    //! @brief The server. Owns the routing, the listener configuration (imutable, keep it right from the start), and
    //! the accept loop definition.
    //!
    //! Not copyable, not movable. `Run()` is `const` and returns a sender, but you shouldn't call it twice. `Stop()`
    //! is thread-safe and idempotent.
    template<
        class Router                              = BasicRouter<Hermes::RawTcpAsyncServer>,
        Hermes::AsyncListenerSocketConcept Socket = Hermes::RawTcpAsyncListener
    >
    struct Server {
        using Data          = typename Socket::DataType;
        using ListenOptions = typename Socket::ListenOptions;
        using AcceptOptions = typename Socket::AcceptOptions;
        using ServerSocket  = typename Socket::ServerSocketType;

        static_assert(RouterForConcept<Router, ServerSocket>, "Invalid router for this socket type.");

        //! @brief Creates a Server. All information passed to the socket is imutable, so they must be correct at the
        //! instantion time.
        template<class = void>
        Server(Data&& data, int backlog = SOMAXCONN)
            requires std::default_initializable<ListenOptions>;

        //! @brief Creates a Server. All information passed to the socket is imutable, so they must be correct at the
        //! instantion time.
        Server(Data&& data, ListenOptions options = {}, int backlog = SOMAXCONN);


        Server(Server&&)                 = delete; // Server is designed to stay on the same scope while it lives.
        Server(const Server&)            = delete; // Server caries definitions that can't be copied.
        Server& operator=(Server&&)      = delete; // Server is designed to stay on the same scope while it lives.
        Server& operator=(const Server&) = delete; // Server caries definitions that can't be copied.

        //! @brief Direct access to the router, so you can add specific behavior.
        Router*       operator->()       noexcept;
        //! @brief Direct access to the router, so you can access specific behavior.
        const Router* operator->() const noexcept;

        //! @brief Start the accept loop. Returns a sender that completes when the server stops, it doesn't block.
        //! @details This sender doesn't produces a value, it just execute the server loop. Given that, the completition
        //! signatures are only `error(ThothError)` (listener failure) and `stopped()` (cancellation via `Stop`).
        //! @return A sender with `error(ThothError)` and `stopped()` channels.
        auto Run() const -> stdexec::sender auto;

        //! @brief Request a graceful stop (idempotent). Does not block.
        void Stop(bool graceful = true, std::chrono::milliseconds deadline = std::chrono::seconds{ 30 }) const;

        //! @brief Returns if the server is running.
        bool Running() const noexcept;

        HttpLimits limits{};
        HostPolicy hosts{};
    private:
        mutable Data  m_data;
        ListenOptions m_options;
        int           m_backlog;

        mutable Router m_router{};

        mutable exec::async_scope m_scope{};
        mutable std::stop_source  m_stop{};
        mutable std::atomic<bool> m_isRunning{ false };

        //! Consecutive accept rejections, the dead-listener detector's streak. Lives on the Server because the
        //! loop's callables are re-connected (and copied) every lap, nothing captured inside them survives one
        //! iteration. Only ever touched by the accept loop, one iteration in flight at a time.
        mutable std::size_t       m_acceptFailureStreak{};


        //! One accepted connection, from the first request byte to the close (or through every keep-alive request
        //! on it). The connection owns the socket and the read window, the exchanges only borrow them.
        auto HandleConnection(ServerSocket conn) const;

        //! One request on a live connection, head to drain. False = the connection is over, for any reason: a
        //! failed head, a failed answer, or a verdict against keep-alive.
        auto HandleOne(ReadContext<ServerSocket>& context, ServerSocket& conn, std::size_t requestCount) const
            -> exec::task<bool>;


        auto WriteResponse(const Exchange<ServerSocket>& request, std::pair<ResponseHead, AnyBodyOut> response,
                ServerSocket& conn, std::size_t requestCount) const noexcept;
    };

}

#include <Thoth/Http/Server/Server.tpp>

#include <Thoth/Http/Client/Client.hpp>
#include <ranges>


namespace rg = std::ranges;
namespace vs = std::views;


// TODO: FUTURE: get from env
constexpr auto downTime { std::chrono::seconds{ 60 } };
constexpr auto sleepTime{ std::chrono::seconds{ 30 } };

Thoth::Http::ClientJanitor & Thoth::Http::ClientJanitor::Instance() {
    static ClientJanitor instance;

    return instance;
}

void Thoth::Http::ClientJanitor::JanitorLoop(std::stop_token stopToken) {
    std::mutex sleepMutex;
    std::condition_variable_any sleepCv;
    std::unique_lock sleepLock{ sleepMutex };

    while (!sleepCv.wait_for(sleepLock, stopToken, sleepTime, [&] { return stopToken.stop_requested(); })) {
        std::lock_guard lock{ poolMutex };
        const auto deadTime{ std::chrono::steady_clock::now() - downTime };

        using SocketPool = decltype(connectionPool)::value_type;
        using Connection = decltype(SocketPool::second)::value_type;

        std::erase_if(connectionPool, [deadTime](SocketPool& connsToEndpoint) {
            std::erase_if(connsToEndpoint.second, [deadTime](const Connection& conn) {
                return conn->lastUsed < deadTime;
            });

            return connsToEndpoint.second.empty();
        });
    };
}




Thoth::Http::ClientJanitor::ClientJanitor() : m_janitorThread{ std::bind_front(&ClientJanitor::JanitorLoop, this) } {}

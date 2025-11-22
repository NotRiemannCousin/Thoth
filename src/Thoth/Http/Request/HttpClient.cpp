#include <Thoth/Http/Client.hpp>
#include <ranges>


namespace rg = std::ranges;
namespace vs = std::views;


// TODO: FUTURE: get from env
constexpr auto downTime{ std::chrono::seconds{60} };
constexpr auto sleepTime{ std::chrono::seconds{30} };

Thoth::Http::ClientJanitor & Thoth::Http::ClientJanitor::Instance() {
    static ClientJanitor instance;

    return instance;
}

void Thoth::Http::ClientJanitor::JanitorLoop() {
    while (_isRunning) {
        std::this_thread::sleep_for(sleepTime);
        std::lock_guard lock(poolMutex);

        const auto deadTime{ std::chrono::steady_clock::now() - downTime };


        std::erase_if(connectionPool, [deadTime](decltype(connectionPool)::value_type& connsToEndpoint) {
            std::erase_if(connsToEndpoint.second, [deadTime](const decltype(connsToEndpoint.second)::value_type& conn) {
                return conn->lastUsed < deadTime;
            });

            return connsToEndpoint.second.empty();
        });
    }
}




Thoth::Http::ClientJanitor::ClientJanitor() {
    _janitorThread = std::jthread(&ClientJanitor::JanitorLoop, this);
}
Thoth::Http::ClientJanitor::~ClientJanitor() {
    _isRunning = false;
    _janitorThread.join();
}


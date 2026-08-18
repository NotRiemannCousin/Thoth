#pragma once 

namespace Thoth::Http {
    template<class T>
    auto ClientConnection::Send(const T& data) {
        return Send(data, SendOptions{});
    }

    template<class T>
    auto ClientConnection::Send(const T& data, SendOptions options) {
        return std::visit([&](auto& sock) {
            using Socket = std::remove_cvref_t<decltype(sock)>;
            typename Socket::SendOptions socketOptions{};
            socketOptions.deadline = options.deadline;
            return sock.Send(data, socketOptions);
        }, socket);
    }
    inline void ClientConnection::Close() {
        std::visit([](auto& sock) { sock.Close(); }, socket);
    }
    inline void ClientConnection::Abort() {
        std::visit([](auto& sock) { sock.Abort(); }, socket);
    }
}


template<>
struct std::hash<Thoth::Http::ClientConnectionKey> {
    size_t operator()(const Thoth::Http::ClientConnectionKey& key) const noexcept {
        size_t seed{ 1469598103934665603ULL };

        Hermes::Utils::HashCombine(seed, std::hash<Hermes::IpEndpoint>{}(key.endpoint));
        Hermes::Utils::HashCombine(seed, std::hash<std::string>{}(key.hostname));
        Hermes::Utils::HashCombine(seed, std::hash<std::string>{}(key.scheme));

        return seed;
    }
};

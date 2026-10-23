// A whole Thoth server in one file, a FastIoLoop, a RawTcpAsyncListener and a SimpleRouter with three routes.
//
// Run it and poke it with curl (the Host policy below accepts both localhost and 127.0.0.1)
//     curl http://localhost:8080/
//     curl http://localhost:8080/hello
//     curl -X POST --data bunnies http://localhost:8080/echo
// then press Enter for a graceful stop, anything the router doesn't know already answers 404 or 405.

#include <iostream>
#include <print>
#include <string>
#include <thread>
#include <utility>

#include <stdexec/execution.hpp>

#include <Thoth/Http/Server/Routers/BasicRouter.hpp>
#include <Thoth/Http/Server/Server.hpp>
#include <Thoth/ThothError.hpp>

int main() {
    Hermes::FastIoLoop ioLoop{ std::thread::hardware_concurrency() };

    Hermes::DefaultSocketData<Hermes::FastIoLoop> listenData{{ Hermes::IpAddress::LoopbackIpv4(), 8080 }};
    listenData.executionContext = &ioLoop;

    Thoth::Http::Server server{ std::move(listenData) };

    // The default Host policy only knows "localhost", a curl poking 127.0.0.1 would eat a 400 before any route
    server.hosts.allowed.push_back("127.0.0.1");

    // ResponseEntry enables automatic conversion from std::string to a Response (with "content-type: plain/text")
    server->Route("/", [](Thoth::Http::GetRequest&) -> Thoth::Http::GetResponseEntry {
        return "hello from Thoth";
    });

    server->Route("/hello", [](Thoth::Http::GetRequest&) -> Thoth::Http::GetResponseEntry {
        return "hi there";
    });

    server->Route("/echo", [](Thoth::Http::PostRequest& request) -> Thoth::Http::PostResponseEntry {
        return std::move(request.body);
    });

    std::println("listening on http://127.0.0.1:8080, press Enter to stop");
    std::println("    curl http://localhost:8080/");
    std::println("    curl http://localhost:8080/hello");
    std::println("    curl -X POST --data bunnies http://localhost:8080/echo");

    std::jthread stopper{ [&server] {
        std::string line;
        std::getline(std::cin, line);
        server.Stop();
    } };

    try {
        // Run() completes through error (the listener died) or stopped (the Enter above), sync_wait wants a
        // value channel so the stop turns into a plain 0.
        stdexec::sync_wait(server.Run() | stdexec::upon_stopped([] { return 0; }));
        std::println("server stopped");
    } catch (const Thoth::ThothError& error) {
        std::println("server died, {}", error);
        std::println("press Enter to leave");
        stopper.join();
        return 1;
    }

    stopper.join();
    return 0;
}
# Thoth
## A functional, expressive, C++26 webdev library

![](Thoth-logo.webp "Thoth, the Egyptian god of writing and wisdom")

Thoth is a modern C++26 lib and webdev library for building both web servers and clients.
Powered by the [Hermes](https://github.com/NotRiemannCousin/Hermes) library, Thoth is designed to harness the latest C++ features
for creating robust, high-performance web applications.
Inspired by the egyptian god of knowledge, magic and the moon, Thoth embraces a philosophy of
strong type safety and compile-time checks without sacrificing usability or elegance.

[//]: # (It heavily)
[//]: # (utilizes coroutines and functional programming concepts to offer a natural and expressive API)
[//]: # (for tasks.)

> CMake says C++23 but is because MSVC is lazy, the target is C++26.

## Installation

### Manual (CMake)

```
include(CPM.cmake)

CPMAddPackage(
        NAME Thoth
        GITHUB_REPOSITORY NotRiemannCousin/Thoth
        GIT_TAG v0.6.0
)

target_link_libraries(your_target PRIVATE Thoth)
```

### pixi

> *Coming soon.*

### vcpkg

> *Maybe, someday, who knows.*

## Server

A Thoth server is a `Server<Router, Hermes::AsyncListenerSocket>`. The class itself manages the connection lifetime, the
accepted hosts, the communication limits. The socket gives you things like execution context + endpoint management and
the Router gives you the route registration, middlewares, fallbacks, etc.

```cpp
#include <print>
#include <thread>

#include <stdexec/execution.hpp>

#include <Thoth/Http/Server/Server.hpp>

int main() {
    Hermes::FastIoLoop ioLoop{ std::thread::hardware_concurrency() };

    Hermes::DefaultSocketData<Hermes::FastIoLoop> listenData{{ Hermes::IpAddress::LoopbackIpv4(), 8080 }};
    listenData.executionContext = &ioLoop;

    Thoth::Http::Server server{ std::move(listenData) };

    // The default Host policy only knows "localhost", curl poking 127.0.0.1 would eat a 400 before any route.
    server.hosts.allowed.push_back("127.0.0.1");

    // The following functions are the behavior of the default router, you can customize it with your own.

    // With ResponseEntry, returning a string becomes a 200 with Content-Type text/plain.
    server->Route("/", [](Thoth::Http::GetRequest&) -> Thoth::Http::GetResponseEntry {
        return "hello from Thoth";
    });

    // JSON responses are automatically serialized with Content-Type application/json.
    server->Route("/json", [](Thoth::Http::GetRequest&) -> Thoth::Http::GetResponseEntry {
        return Thoth::NJson::Json{{ { "hello", "hello from Thoth" } }};
    });

    // Bodies round-trip, strings, bytes, JSON and files.
    server->Route("/echo", [](Thoth::Http::PostRequest& request) -> Thoth::Http::PostResponseEntry {
        return std::move(request.body);
    });

    // One path, one callback per method (404 and 405 are set by the BasicRouter).
    server->Match("/res",
        [](Thoth::Http::PutRequest&)    -> Thoth::Http::ResponseEntry<Thoth::Http::PutMethod>    { return "put"; },
        [](Thoth::Http::DeleteRequest&) -> Thoth::Http::ResponseEntry<Thoth::Http::DeleteMethod> { return "deleted"; });

    std::println("listening on http://127.0.0.1:8080, press Enter to stop");
    std::jthread stopper{ [&server] {
        std::string line;
        std::getline(std::cin, line);
        server.Stop();
    } };

    // Run() is a sender: it completes on listener error or when Stop() cancels it.
    stdexec::sync_wait(server.Run() | stdexec::upon_stopped([] { return 0; }));
}
```

With BasicRouter, routes can carry middlewares through `Use(...)`. A middleware call must return `std::optional<T>` or
`std::expected<T, IntoResponsePayload>`. If a route declares a parameter `T` it will see the middleware as required and
will fail the request if it fails (`std::expected` with the given response from the payload, `std::optional` with
`400 BAD REQUEST`). If it declares `std::optional<T>` the middleware can fail because it will be passed as
`std::nullopt`,
The server is also exception safe, handlers can throw a `StatusCodeEnum` to answer it directly.

```cpp
struct Session { std::string user; };

server->Route("/who", Thoth::Http::Use(
        [](const auto&) -> std::expected<Session, Thoth::Http::StatusCodeEnum> {
            return Session{ "thoth" };
        }),
    [](const Thoth::Http::GetRequest&, Session who) -> Thoth::Http::GetResponseEntry {
        return std::format("user:{}", who.user);
    });
```

## Client

The client has a very straightforward functional API:

```cpp
#include <print>
#include <Thoth/Http/Client/Client.hpp>

namespace NHttp = Thoth::Http;
namespace NJson = Thoth::NJson;
using NJson::Json;

std::expected<std::vector<Json>, Thoth::ThothError> GetMembers(size_t id) {
    using std::string_literals::operator ""s;
    namespace Utils = Thoth::Utils;

    //trying to make the request, send to the server and then convert the body to JSON.
    return NHttp::GetRequest::FromUrl(std::format("https://api.discogs.com/artists/{}", id))
            // All these functions have the same error type `ThothError`, a std::variant with each specific error.
            .and_then(NHttp::Client::H_Send())
            .and_then(&NHttp::Response<>::AsJson<>)

            // selecting "members" in the first object
            .and_then(std::bind_back(&Json::GetAndMoveOrError, "members" ))

            // making sure that it's an array
            .and_then(&Json::EnsureMovOrError<NJson::Array>);
}



int main() {
    static constexpr auto getName{ [](const Json& member) {
        return member.Get("name")
                .and_then(&Json::EnsureRef<NJson::String>)
                .transform(&NJson::String::AsCopy) // converting from Thoth Strings (that are COW) to std::string
                .value_or("<unnamed>");
    } };

    static constexpr auto printNames{ [](auto&& names) {
        std::println("- Members:");
        for (std::string&& name : names)
            std::println("{}", name);

        return std::monostate{};
    } };

    static constexpr auto errorHandler{ [](auto&& error) {
        std::println("An error occurred: {}", error);

        if (const auto err{ Hermes::GetError() }; !err)
            std::println("OS error: {}", err.error());

        return std::monostate{};
    } };




    GetMembers(4001234)
            .transform(std::views::transform(getName))
            .transform(printNames)
            .transform_error(errorHandler);

    return 0;
}
```

## Middlewares

Client requests can be wrapped with composable middlewares. Each one takes a handler
(`Request<Method, Body> -> ExpResponse<Method, Body>`, the same shape as `Client::H_Send()`) and
returns a new handler, so they nest around the base send call:

```cpp
NHttp::GetRequest::FromUrl(url)
    .and_then(NHttp::FollowRedirects(NHttp::Retry(3, NHttp::Decompress(NHttp::Client::H_Send()))))
```

Available today: `FollowRedirects` (307/308), `FollowSeeOther` (303), `Retry` (idempotent methods only, exponential backoff with configurable support to `Retry-After` header), and `Decompress` (gzip/deflate). You can define your ownmiddlewares too.

## Examples

Ready examples in the `examples/` directory:

- `BasicServer`: the server above, whole and minimal.
- `PostMortemServer`: a "simple" textboard.
- `Localhost`, `BodyToFile1/2`, `Request1/2` and the JSON ones: client-side usage.
- `Comparison`: comparison against other libraries (needs `-DTHOTH_BUILD_COMP_EXAMPLE=ON`).

## Requirements

- Windows 10 or newer / Linux

- OpenSSL and liburing on Linux (`sudo apt install libssl-dev liburing-dev`)
- MSVC or GCC with C++26 support (GCC/Clang on Linux)
- CMake 3.29.1 or newer
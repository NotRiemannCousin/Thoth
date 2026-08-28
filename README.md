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

> Server side connections are under development, only client connections are availabe.
> CMake says C++23 but is because MSVC is lazy, the target is C++26.

## Middlewares

Client requests can be wrapped with composable middlewares. Each one takes a handler
(`Request<Method, Body> -> ExpResponse<Method, Body>`, the same shape as `Client::H_Send()`) and
returns a new handler, so they nest around the base send call:

```cpp
NHttp::GetRequest::FromUrl(url)
    .and_then(NHttp::FollowRedirects(NHttp::Retry(3, NHttp::Decompress(NHttp::Client::H_Send()))))
```

Available today: `FollowRedirects` (307/308), `FollowSeeOther` (303), `Retry` (idempotent methods only, exponential
backoff with configurable support to `Retry-After` header), and `Decompress` (gzip/deflate). You can define your own
middlewares too.

## Examples
```cpp
namespace NHttp = Thoth::Http;
namespace NJson = Thoth::NJson;
using NJson::Json;

std::expected<std::vector<Json>, std::string> GetMembers(size_t id) {
    using std::string_literals::operator ""s;
    namespace Utils = Thoth::Utils;

    //trying to make the request, send to the server and then convert the body to JSON.
    return NHttp::GetRequest::FromUrl(std::format("https://api.discogs.com/artists/{}", id))
            .and_then(NHttp::Client::H_Send())
            .and_then(&NHttp::GetResponse::AsJson)

            // selecting "members" in the first object
            .transform(std::bind_back(&Json::GetAndMove, "members" ))

            // ensuring that it is an array
            .transform(&Json::EnsureMov<NJson::Array>)
            .and_then(Utils::ValueOrHof<NJson::Array>("'members' array doesn't exist."s));
}



int main() {
    static constexpr auto getName{ [](const Json& member) {
        return member.Get("name")
                .and_then(&Json::EnsureRef<NJson::String>)
                .transform(&NJson::String::AsCopy) // converting from Thoth Strings (that are COW) to std::string
                .value_or("<unnamed>");
    } };

    static auto constexpr printNames{ [](auto&& names) {
        std::println("- Members:");
        for (std::string&& name : names)
            std::println("{}", name);

        return std::monostate{};
    } };

    static auto constexpr errorHandler{ [](auto&& error) {
        std::println("An error occurred: {}", error);

        if (const int wsaError{ WSAGetLastError() }; wsaError != 0)
            std::println("WSA error: {}", wsaError);

        return std::monostate{};
    } };


    GetMembers(4001234)
            .transform(std::views::transform(getName))
            .transform(printNames)
            .transform_error(errorHandler);

    return 0;
}

```
# Thoth
## A functional, expressive, asynchronous C++26 webdev library

![](Thoth-logo.webp "Thoth, the Egyptian god of writing and wisdom")

Thoth is a modern C++26 lib and webdev library for building both web servers and clients.
Powered by the [Hermes](https://github.com/NotRiemannCousin/Hermes) library, Thoth is designed to harness the latest C++ features
for creating robust, high-performance web applications.

Inspired by the egyptian god of knowledge, magic and the moon, Thoth embraces a philosophy of
strong type safety and compile-time checks without sacrificing usability or elegance. It heavily
utilizes coroutines and functional programming concepts to offer a natural and expressive API
for asynchronous tasks.


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
            // At the current moment every error in this string is given by a string, but it will be changed
            // to enums/proper structs in the future.
            .and_then(NHttp::Client::Send<>)
            .and_then(&NHttp::GetResponse::AsJson)

            // selecting "members" in the first object
            .transform(std::bind_back(&Json::GetAndMove, "members" ))

            // ensuring that it is an array
            .transform(&Json::EnsureMov<NJson::Array>)
            .and_then(Utils::ValueOrHof<NJson::Array>("'members' array doesn't exist."s));
}



int main() {
    static constexpr auto s_getName = [](const Json& member) {
        return member.Get("name")
                .and_then(&Json::EnsureRef<NJson::String>)
                .transform(&NJson::String::AsCopy) // converting from Thoth Strings (that are COW) to std::string
                .value_or("<unnamed>");
    };

    static auto constexpr s_printNames = [](auto&& names) {
        std::println("- Members:");
        for (string&& name : names)
            std::println("{}", name);

        return std::monostate{};
    };

    static auto constexpr s_errorHandler = [](auto&& error) {
        std::println("An error occurred: {}", error);

        if (const int wsaError{ WSAGetLastError() }; wsaError != 0)
            std::println("WSA error: {}", wsaError);

        return std::monostate{};
    };


    GetMembers(4001234)
            .transform(std::views::transform(s_getName))
            .transform(s_printNames)
            .transform_error(s_errorHandler);

    return 0;
}

```
# Thoth
## An expressive, asynchronous C++26 webdev library

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
std::expected<std::monostate, std::string> FunctionalRequest() {
    using std::string_literals::operator ""s;
    namespace Utils = Thoth::Utils;

    constexpr auto s_printOrDie = [](const NJson::Array& members) -> std::expected<std::monostate, std::string> {
        for (const auto& member : members) {
            if (!member.IsOf<NJson::Object>())
                return std::unexpected{ R"(Value isn't an object)" };

            std::print("{}\n", (*member.As<NJson::Object>())["name"]);
        }

        return std::monostate{};
    };

    return NHttp::GetRequest::FromUrl("https://api.discogs.com/artists/4001234")
            .transform(NHttp::Client::Send<>)
            .value_or(std::unexpected{ "Failed to connect." })
            .transform(&NHttp::GetResponse::AsJson)
            .and_then(Utils::ValueOrHof<Json>("Cant convert to json."s))

            .transform(std::bind_back(&Json::GetAndMove, "members" ))

            .transform(&Json::EnsureMov<NJson::Array>)
            .and_then(Utils::ValueOrHof<NJson::Array>("'members' array doesn't exist."s))

            .and_then(s_printOrDie);
}
```
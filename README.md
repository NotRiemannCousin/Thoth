# Thoth
## An expressive, asynchronous C++26 web microframework

![](Thoth-logo.webp "Thoth, the Egyptian god of writing and wisdom")

Thoth is a modern C++26 lib and microframework for building both web servers and clients.
Powered by the [Hermes](https://github.com/NotRiemannCousin/Hermes) library, Thoth is designed to harness the latest C++ features
for creating robust, high-performance web applications.

Inspired by the egyptian god of knowledge, magic and the moon, Thoth embraces a philosophy of
strong type safety and compile-time checks without sacrificing usability or elegance. It heavily
utilizes coroutines and functional programming concepts to offer a natural and expressive API
for asynchronous tasks.


## Examples
### Functional Version 1
```cpp
std::expected<std::monostate, std::string> FunctionalRequest() {
    using std::string_literals::operator ""s;
    namespace Utils = Thoth::Utils;

    const auto membersOrError {
        NHttp::GetRequest::FromUrl("https://api.discogs.com/artists/4001234")
                .transform(NHttp::HttpClient::Send<>)
                .value_or(std::unexpected{ "Failed to connect." })
                .transform(&NHttp::GetResponse::AsJson)
                .and_then(Utils::ValueOrHof<std::optional<Json>&&>("Cant convert to json."s))
                .transform(std::bind_back(&Json::GetAndMove, "members" ))
                .and_then(Utils::ValueOrHof<NJson::OptValWrapper&&>("\"members\" doesn't exist."s))
                .and_then(Utils::ErrorIfNotHof<&Json::IsOf<NJson::Array>, Json>("\"members\" isn't an array."s))
    };

    if (!membersOrError)
        return std::unexpected{ membersOrError.error() };

    for (const auto& member : membersOrError->As<NJson::Array>()) {
        if (!member.IsOf<NJson::Object>())
            return std::unexpected{ "Value isn't an object" };

        std::print("{}\n", (*member.As<NJson::Object>())["name"]);
    }

    return {};
}

```
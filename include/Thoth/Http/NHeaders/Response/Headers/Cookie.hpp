#pragma once
#include <optional>
#include <string>
#include <chrono>


namespace Thoth::Http::NHeaders {
    enum class SameSiteEnum { None, Strict, Lax };

    struct Cookie {
        using Date = std::chrono::system_clock::time_point;
        std::string name{};
        std::string value{};
        std::optional<Date>         expires{};
        std::optional<int64_t>      maxAge{};
        std::optional<std::string>  domain{};
        std::optional<std::string>  path{};
        std::optional<SameSiteEnum> sameSite{};
        bool partitioned{};
        bool secure{};
        bool httpOnly{};
    };

}

#include <Thoth/Http/NHeaders/Response/Headers/Cookie.tpp>

static_assert(Thoth::Utils::Serializable<Thoth::Http::NHeaders::Cookie>);
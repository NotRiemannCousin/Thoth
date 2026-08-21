// Response/Headers/Challenge.hpp
#pragma once
#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <string_view>
#include <algorithm>

namespace Thoth::Http::NHeaders {
    //! @brief One "WWW-Authenticate"/"Proxy-Authenticate" challenge (RFC 7235 §4.1).
    //!
    //! @note Params are stored unescaped, as key/value pairs. Token68 challenges
    //! (schemes with no `key=value` params, e.g. some "Negotiate" tokens) are stored
    //! as a single param named "token68".
    struct Challenge {
        std::string scheme{};
        std::vector<std::pair<std::string, std::string>> params{};

        [[nodiscard]] std::optional<std::string_view> Param(std::string_view key) const {
            const auto it{ std::ranges::find_if(params, [&](const auto& p) {
                return std::ranges::equal(p.first, key, String::CaseInsensitiveCompare);
            }) };
            if (it == params.end()) return std::nullopt;
            return it->second;
        }
    };
}

#include <Thoth/Http/NHeaders/Response/Headers/Challenge.tpp>


static_assert(Thoth::Utils::Serializable<Thoth::Http::NHeaders::Challenge>);
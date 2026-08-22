// Response/Headers/Challenge.hpp
#pragma once
#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <string_view>
#include <algorithm>

namespace Thoth::Http::NHeaders {
    //! @brief One authentication challenge from `WWW-Authenticate` or `Proxy-Authenticate` (RFC 9110 §§11.6.1, 11.7.1).
    //!
    //! `scheme` stores the authentication scheme and `params` stores its parsed  parameters as unescaped key/value
    //! pairs. A token68 challenge, such as  `Negotiate a874bg==`, is represented by a parameter named `token68`.
    //!
    //! @par Example
    //! @code{.cpp}
    //! Challenge challenge{
    //!     "Digest",
    //!     {{ "realm", "users" }, { "qop", "auth" }}
    //! };
    //! auto realm{ challenge.Param("realm") };
    //! @endcode
    struct Challenge {
        //! Authentication scheme, for example `Basic`, `Bearer` or `Digest`.
        std::string scheme{};
        //! Parsed unescaped challenge parameters in wire order.
        std::vector<std::pair<std::string, std::string>> params{};

        //! @brief Looks up a challenge parameter by name.
        //! @param key Parameter name, compared case-insensitively.
        //! @return The parameter value, or `std::nullopt` when it is absent.
        [[nodiscard]] std::optional<std::string_view> Param(std::string_view key) const;
    };
}

#include <Thoth/Http/NHeaders/Response/Headers/Challenge.tpp>


static_assert(Thoth::Utils::Serializable<Thoth::Http::NHeaders::Challenge>);
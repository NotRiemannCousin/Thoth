#pragma once
#include <optional>
#include <string>
#include <chrono>


namespace Thoth::Http::NHeaders {
    //! @brief SameSite delivery policy for a response cookie (draft RFC 6265bis).
    //!
    //! The value is serialized as `SameSite=None`, `SameSite=Strict` or `SameSite=Lax` when the corresponding optional
    //! field is present.
    enum class SameSiteEnum {
        //! Send the cookie in cross-site contexts when other requirements allow it.
        None,
        //! Send the cookie only in same-site contexts.
        Strict,
        //! Use the browser's lax cross-site delivery policy.
        Lax
    };

    //! @brief A `Set-Cookie` response field value (RFC 6265 §§4.1, 4.2).
    //!
    //! The first two fields form the required `name=value` pair. The remaining fields model common cookie attributes,
    //! an unset optional attribute is not emitted. Boolean attributes such as `Secure` and `HttpOnly` are emitted when
    //! their flags are `true`.
    //!
    //! @par Example
    //! @code{.cpp}
    //! Cookie session{
    //!     .name = "session",
    //!     .value = "abc",
    //!     .path = "/",
    //!     .sameSite = SameSiteEnum::Lax,
    //!     .secure = true,
    //!     .httpOnly = true
    //! };
    //! @endcode
    struct Cookie {
        //! Alias for the time point used by the cookie expiry attribute.
        using Date = std::chrono::system_clock::time_point;
        //! Cookie name from the `name=value` pair.
        std::string name{};
        //! Cookie value from the `name=value` pair.
        std::string value{};
        //! Optional absolute expiration time.
        std::optional<Date>         expires{};
        //! Optional lifetime in seconds.
        std::optional<int64_t>      maxAge{};
        //! Optional domain attribute.
        std::optional<std::string>  domain{};
        //! Optional path attribute.
        std::optional<std::string>  path{};
        //! Optional SameSite delivery policy.
        std::optional<SameSiteEnum> sameSite{};
        //! Whether the `Partitioned` attribute is emitted.
        bool partitioned{};
        //! Whether the `Secure` attribute is emitted.
        bool secure{};
        //! Whether the `HttpOnly` attribute is emitted.
        bool httpOnly{};
    };

}

#include <Thoth/Http/NHeaders/Response/Headers/Cookie.tpp>

static_assert(Thoth::Utils::Serializable<Thoth::Http::NHeaders::Cookie>);
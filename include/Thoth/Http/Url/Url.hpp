#pragma once
#include <expected>
#include <filesystem>
#include <string>
#include <Hermes/Endpoint/IpEndpoint/IpAddress.hpp>
#include <Thoth/Http/Request/QueryParams.hpp>
#include <Thoth/Utils/Scanner.hpp>


namespace Thoth::Http {
    using Host = std::variant<std::string_view, Hermes::IpAddress>;

    struct Authority {
        std::string_view userinfo{};
        Host host;
        std::optional<std::uint16_t> port{};

        [[nodiscard]] std::string GetHostString() const;
    };

    std::optional<std::uint16_t> GetDefaultPort(std::string_view scheme) noexcept;

    struct Url {

        Url(const Url& other);
        Url(Url&& other) noexcept;

        Url& operator=(const Url& other);
        Url& operator=(Url&& other) noexcept;

        [[nodiscard]] std::string_view         GetScheme()    const noexcept;
        [[nodiscard]] std::optional<Authority> GetAuthority() const noexcept;
        [[nodiscard]] std::string_view         GetPath()      const noexcept;
        [[nodiscard]] std::string_view         GetQuery()     const noexcept;
        [[nodiscard]] std::string_view         GetFragment()  const noexcept;

        // ok Im not of how to implement this yet, but std::filesystem::path seams to an abuse
        // [[nodiscard]] std::optional<std::filesystem::path> GetPathAsFilePath() const;

        [[nodiscard]] std::string_view         GetPathOrSep()          const noexcept;
        [[nodiscard]] QueryParams              GetQueryParams()        const;
        [[nodiscard]] std::string_view         GetUrlWithoutFragment() const noexcept;

        //! @brief Tries to convert the given string into URL.
        //! @param rawUrl the given URL.
        //! @return The Url if succeeded, std::nullopt if it fails.
        static std::expected<Url, ExchangeError> FromUrl(std::string rawUrl);
        //! @brief Resolves a URI reference against this URL as base, per RFC 3986 §5.2.2.
        //!
        //! Supports absolute references (returned verbatim as a parsed @ref Url),
        //! relative-path references (@c "../v2/users"), same-document references (@c ""),
        //! and fragment-only references (@c "#section").
        //!
        //! @param reference A URI reference. Does not need to be absolute.
        //! @return The resolved absolute @ref Url, or an @ref ExchangeError if the
        //! resolved result fails parsing (e.g. produces an invalid scheme).
        [[nodiscard]] std::expected<Url, ExchangeError> Resolve(std::string_view reference) const;

        //! @brief Static version of @ref Resolve.
        static std::expected<Url, ExchangeError> ResolveRelative(const Url& url, std::string_view reference);

        //! @brief Encodes a text with <a href="https://datatracker.ietf.org/doc/html/rfc3986#section-2.1">
        //! Percent-Encoding</a>.
        //! @param str the given text.
        //! @return The string encoded.
        static std::string Encode(std::string_view str);
        //! @brief Tries to decode the text with <a href="https://datatracker.ietf.org/doc/html/rfc3986#section-2.1">
        //! Percent-Encoding</a>.
        //! @param str the given text.
        //! @return The string decoded if it succeeded, std::nullopt if it fails.
        static std::expected<std::string, ExchangeError> TryDecode(std::string_view str);

        bool operator==(const Url& other) const noexcept;
    private:
        friend struct UrlBuilder;
        friend struct std::formatter<Url>;
        friend struct std::hash<Url>;

        explicit Url() = default;

        std::string              m_rawUrl{};
        std::string_view         m_scheme{};
        std::optional<Authority> m_authority{};
        std::string_view         m_path{};
        std::string_view         m_query{};
        std::string_view         m_frag{};
    };
}


#include <Thoth/Http/Url/Url.tpp>

static_assert(Thoth::Utils::Serializable<Thoth::Http::Url>);
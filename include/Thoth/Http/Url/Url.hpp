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
        static std::expected<Url, RequestError> FromUrl(std::string rawUrl);


        //! @brief Encodes a text with <a href="https://datatracker.ietf.org/doc/html/rfc3986#section-2.1">
        //! Percent-Encoding</a>.
        //! @param str the given text.
        //! @return The string encoded.
        static std::string Encode(std::string_view str);
        //! @brief Tries to decode the text with <a href="https://datatracker.ietf.org/doc/html/rfc3986#section-2.1">
        //! Percent-Encoding</a>.
        //! @param str the given text.
        //! @return The string decoded if it succeeded, std::nullopt if it fails.
        static std::expected<std::string, RequestError> TryDecode(std::string_view str);

        bool operator==(const Url& other) const noexcept;
    private:
        friend struct UrlBuilder;
        friend struct std::formatter<Url>;
        friend struct std::hash<Url>;

        explicit Url() = default;

        std::string rawUrl{};
        std::string_view scheme{};
        std::optional<Authority> authority{};
        std::string_view path{};
        std::string_view query{};
        std::string_view frag{};
    };
}


#include <Thoth/Http/Url/Url.tpp>

static_assert(Thoth::Utils::Serializable<Thoth::Http::Url>);
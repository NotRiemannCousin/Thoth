#pragma once
#include <expected>
#include <filesystem>
#include <string>
#include <Hermes/Endpoint/IpEndpoint/IpAddress.hpp>
#include <Thoth/Http/Request/QueryParams.hpp>
#include <Thoth/Utils/Scanner.hpp>


namespace Thoth::Http {
    using Host     = std::variant<std::string, Hermes::IpAddress>;
    using HostView = std::variant<std::string_view, Hermes::IpAddress>;


    struct Authority {
        std::string userinfo{};
        Host host;
        std::optional<std::uint16_t> port{};

        [[nodiscard]] std::string GetHostString() const;
    };

    struct AuthorityView {
        std::string_view userinfo{};
        HostView host;
        std::optional<std::uint16_t> port{};

        [[nodiscard]] std::string GetHostString() const;
    };

    std::optional<std::uint16_t> GetDefaultPort(std::string_view scheme) noexcept;

    //! @brief A subset of URL as defined in <a href="https://datatracker.ietf.org/doc/html/rfc3986">RFC 3986</a>.
    //!
    //! @note Url is an immutable record: once parsed via @ref FromUrl, its components  cannot be mutated in place.
    //!
    //! @warning Views returned by this class are only valid for the lifetime of *this* Url instance, and are
    //! invalidated by move (the moved-from Url no longer owns the buffer). Do not store an @ref AuthorityView
    //! or any `string_view` obtained from this class past the lifetime of the Url, uses the Copy methods instead.
    struct Url {
        using AuthorityViewOpt = std::optional<AuthorityView>;
        using AuthorityOpt     = std::optional<Authority>;
        using RangeIdx         = std::pair<std::size_t, std::size_t>;


        Url(const Url& other) = default;
        Url(Url&& other) noexcept = default;

        Url& operator=(const Url& other) = default;
        Url& operator=(Url&& other) noexcept = default;

        //! @brief returns a reference to the URL Scheme.
        [[nodiscard]] std::string_view GetScheme()    const noexcept;
        //! @brief returns a reference to the URL Authority.
        [[nodiscard]] AuthorityViewOpt GetAuthority() const noexcept;
        //! @brief returns a reference to the URL Path.
        [[nodiscard]] std::string_view GetPath()      const noexcept;
        //! @brief returns a reference to the URL Query.
        [[nodiscard]] std::string_view GetQuery()     const noexcept;
        //! @brief returns a reference to the URL Fragment.
        [[nodiscard]] std::string_view GetFragment()  const noexcept;


        //! @brief returns a copy of the URL Scheme.
        [[nodiscard]] std::string  CopyScheme()    const noexcept;
        //! @brief returns a copy of the URL Authority.
        [[nodiscard]] AuthorityOpt CopyAuthority() const noexcept;
        //! @brief returns a copy of the URL Path.
        [[nodiscard]] std::string  CopyPath()      const noexcept;
        //! @brief returns a copy of the URL Query.
        [[nodiscard]] std::string  CopyQuery()     const noexcept;
        //! @brief returns a copy of the URL Fragment.
        [[nodiscard]] std::string  CopyFragment()  const noexcept;

        // ok Im not of how to implement this yet, but std::filesystem::path seams to an abuse
        // [[nodiscard]] std::optional<std::filesystem::path> GetPathAsFilePath() const;

        //! @brief return the "/{path}" (always shows "/").
        [[nodiscard]] std::string_view GetPathOrSep()          const noexcept;
        //! @brief Parses the query as QueryParams.
        [[nodiscard]] QueryParams      GetQueryParams()        const;
        //! @brief It... returns the URL without fragment. What did you expected?
        [[nodiscard]] std::string_view GetUrlWithoutFragment() const noexcept;

        //! @brief Tries to convert the given string into URL.
        //! @param rawUrl the given URL.
        //! @return The Url if succeeded, std::nullopt if it fails.
        static std::expected<Url, ThothError> FromUrl(std::string rawUrl);
        //! @brief Resolves a URI reference against this URL as base, per RFC 3986 §5.2.2.
        //!
        //! Supports absolute references (returned verbatim as a parsed @ref Url),
        //! relative-path references (@c "../v2/users"), same-document references (@c ""),
        //! and fragment-only references (@c "#section").
        //!
        //! @param reference A URI reference. Does not need to be absolute.
        //! @return The resolved absolute @ref Url, or an @ref ThothError if the
        //! resolved result fails parsing (e.g. produces an invalid scheme).
        [[nodiscard]] std::expected<Url, ThothError> Resolve(std::string_view reference) const;

        //! @brief Static version of @ref Resolve.
        static std::expected<Url, ThothError> ResolveRelative(const Url& url, std::string_view reference);

        //! @brief Encodes a text with <a href="https://datatracker.ietf.org/doc/html/rfc3986#section-2.1">
        //! Percent-Encoding</a>.
        //! @param str the given text.
        //! @return The string encoded.
        static std::string Encode(std::string_view str);
        //! @brief Tries to decode the text with <a href="https://datatracker.ietf.org/doc/html/rfc3986#section-2.1">
        //! Percent-Encoding</a>.
        //! @param str the given text.
        //! @return The string decoded if it succeeded, std::nullopt if it fails.
        static std::expected<std::string, ThothError> TryDecode(std::string_view str);

        bool operator==(const Url& other) const noexcept;
    private:
        struct AuthorityIdx {
            RangeIdx userinfo{};
            std::variant<RangeIdx, Hermes::IpAddress> host;
            std::optional<std::uint16_t> port{};
        };

        friend struct UrlBuilder;
        friend struct std::formatter<Url>;
        friend struct std::hash<Url>;

        explicit Url() = default;

        [[nodiscard]] std::string_view View(RangeIdx range) const noexcept;
        [[nodiscard]] static RangeIdx MakeRange(const std::string& source, std::string_view view) noexcept;

        std::string m_rawUrl{};

        RangeIdx                      m_schemeIdx{};
        std::optional<AuthorityIdx>   m_authority{};
        RangeIdx                      m_pathIdx{};
        std::optional<RangeIdx>       m_queryIdx{};
        std::optional<RangeIdx>       m_fragIdx{};
    };
}


#include <Thoth/Http/Url/Url.tpp>

static_assert(Thoth::Utils::Serializable<Thoth::Http::Url>);

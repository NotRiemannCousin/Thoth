#pragma once
#include <string>
#include <Thoth/Http/Request/QueryParams.hpp>

namespace Thoth::Http {
    using string_view = std::string_view;
    using string = std::string;

    struct Url {
        //! @brief the given scheme. Can be "http" or "https" (it's Url after all).
        string scheme{};
        //! @brief the userinfo of the url, the optional part between the scheme and the host.
        //! eg: https://userinfo@@localhost.
        string user{};
        //! @brief the hostname of the url.
        string host{};
        //! @brief the port of the url. Goes from 0 to 65.535.
        int port{};
        //! @brief the path of the url.
        string path{};
        //! @brief the query of the url.
        QueryParams query{};
        //! @brief the fragment of the url. Normally ignored in server side.
        string fragment{};

        //! @brief Tries to convert the given string into Url.
        //! @param rawUrl the given url.
        //! @return The Url if succeeded, std::nullopt if it fails.
        static std::optional<Url> FromUrl(string_view rawUrl);


        //! @brief Encodes a text with <a href="https://datatracker.ietf.org/doc/html/rfc3986#section-2.1">
        //! Percent-Encoding</a>.
        //! @param str the given text.
        //! @return The string encoded.
        static string Encode(string_view str);
        //! @brief Tries to decode the text with <a href="https://datatracker.ietf.org/doc/html/rfc3986#section-2.1">
        //! Percent-Encoding</a>.
        //! @param str the given text.
        //! @return The string decoded if it succeeded, std::nullopt if it fails.
        static std::optional<string> TryDecode(string_view str);

        //! @brief Check if the scheme is https.
        [[nodiscard]] bool IsSecure() const;

        bool operator==(const Url &) const;

    private:
        Url();

        friend class std::formatter<Url>;
    };
}


#include <Thoth/Http/Request/Url.tpp>
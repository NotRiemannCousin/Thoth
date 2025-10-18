#pragma once
#include <string>
#include <Thoth/Http/Request/QueryParams.hpp>

namespace Thoth::Http {
    using string_view = std::string_view;
    using string = std::string;

    struct HttpUrl {
        //! @brief the given scheme. Can be "http" or "https" (it's HttpUrl after all).
        string scheme{};
        //! @brief the userinfo of the url, the optional part between the scheme and the host.
        //! eg: https://userinfo@@localhost.
        string user{};
        //! @brief the hostname of the url.
        string host{};
        //! @brief the path of the url.
        string path{};
        //! @brief the port of the url. Goes from 0 to 65.535.
        int port{};
        //! @brief the query of the url.
        QueryParams query{};
        //! @brief the fragment of the url. Normally ignored in server side.
        string fragment{};

        //! @brief Tries to convert the given string into HttpUrl.
        //! @param url the given url.
        //! @return The HttpUrl if succeeded, std::nullopt if it fails.
        static std::optional<HttpUrl> FromUrl(string_view url);


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
        bool IsSecure() const;

        bool operator==(const HttpUrl &) const;

    private:
        HttpUrl();

        friend class std::formatter<HttpUrl>;
    };
}


#include <Thoth/Http/Request/HttpUrl.tpp>
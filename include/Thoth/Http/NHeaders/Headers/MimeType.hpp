#pragma once
#include <string>
#include <optional>
#include <Thoth/Http/NHeaders/_base.hpp>

namespace Thoth::Http::NHeaders {
    struct MimeType {
        std::string type{};
        std::string subtype{};
        std::vector<std::pair<std::string, std::string>> options{};

        bool operator==(const MimeType& other) const;
    };

    struct MimeTypes {
        MimeTypes() = delete;

        inline static const MimeType textPlain      { "text",        "plain" };
        inline static const MimeType textHtml       { "text",        "html" };
        inline static const MimeType appJson        { "application", "json" };
        inline static const MimeType appXml         { "application", "xml" };
        inline static const MimeType appOctetStream { "application", "octet-stream" };
        inline static const MimeType imagePng       { "image",       "png" };
        inline static const MimeType imageJpeg      { "image",       "jpeg" };
        inline static const MimeType multipartForm  { "multipart",   "form-data" };
    };
}

#include <Thoth/Http/NHeaders/Headers/MimeType.tpp>

static_assert(Thoth::Utils::Serializable<Thoth::Http::NHeaders::MimeType>);
#pragma once
#include <optional>
#include <Thoth/Http/NHeaders/_base.hpp>

namespace Thoth::Http::NHeaders {
    struct MimeType {
        string type;
        string subtype;
        std::vector<std::pair<string, string>> options{};
    };

    struct MimeTypes {
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

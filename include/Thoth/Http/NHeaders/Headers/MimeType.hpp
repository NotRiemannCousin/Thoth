#pragma once
#include <string>
#include <optional>
#include <Thoth/Http/NHeaders/_base.hpp>
#include <Thoth/Http/NHeaders/_base/Parameterized.hpp>

namespace Thoth::Http::NHeaders {
    struct MimeTypeHeader {
        std::string type{};
        std::string subtype{};

        bool operator==(const MimeTypeHeader&) const = default;
    };
}
#include <Thoth/Http/NHeaders/Headers/MimeType.tpp>

namespace Thoth::Http::NHeaders {
    using MimeType = Parameterized<MimeTypeHeader>;

    static_assert(Thoth::Utils::Serializable<MimeTypeHeader>);

    struct MimeTypes {
        MimeTypes() = delete;
        inline static const MimeType textPlain      { MimeTypeHeader{ "text"       , "plain"        } };
        inline static const MimeType textHtml       { MimeTypeHeader{ "text"       , "html"         } };
        inline static const MimeType appJson        { MimeTypeHeader{ "application", "json"         } };
        inline static const MimeType appXml         { MimeTypeHeader{ "application", "xml"          } };
        inline static const MimeType appOctetStream { MimeTypeHeader{ "application", "octet-stream" } };
        inline static const MimeType imagePng       { MimeTypeHeader{ "image"      , "png"          } };
        inline static const MimeType imageJpeg      { MimeTypeHeader{ "image"      , "jpeg"         } };
        inline static const MimeType multipartForm  { MimeTypeHeader{ "multipart"  , "form-data"    } };
    };

}
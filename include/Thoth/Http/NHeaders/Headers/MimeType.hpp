#pragma once
#include <string>
#include <optional>
#include <Thoth/Http/NHeaders/_base.hpp>
#include <Thoth/Http/NHeaders/_base/Parameterized.hpp>

namespace Thoth::Http::NHeaders {
    //! @brief The `type/subtype` portion of a media type (RFC 9110 §8.3.1).
    //!
    //! Examples include `text/plain` and `application/json`. Parameters such as `charset` belong to the `MimeType`
    //! wrapper rather than this struct.
    //!
    //! @par Example
    //! @code{.cpp}
    //! MimeTypeHeader json{ "application", "json" };
    //! @endcode
    struct MimeTypeHeader {
        std::string type{};
        std::string subtype{};

        bool operator==(const MimeTypeHeader&) const = default;
    };
}
#include <Thoth/Http/NHeaders/Headers/MimeType.tpp>

namespace Thoth::Http::NHeaders {
    //! @brief Parameterized version of `MimeTypeHeader`.
    //! @copydoc MimeTypeHeader
    using MimeType = Parameterized<MimeTypeHeader>;

    static_assert(Thoth::Utils::Serializable<MimeTypeHeader>);

    //! @brief Common media type constants for proxy assignment and comparison.
    //!
    //! The constants contain only the base `type/subtype`, add parameters with `MimeType::WithParam()` when needed.
    //! @par Example
    //! @code{.cpp}
    //! auto json{ MimeTypes::appJson.WithParam("charset", "utf-8") };
    //! @endcode
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
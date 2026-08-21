#pragma once
#include <string>
#include <Thoth/Http/NHeaders/_base.hpp>
#include <Thoth/Http/NHeaders/_base/Parameterized.hpp>

namespace Thoth::Http::NHeaders {
    //! @brief The disposition-type token of a Content-Disposition header (RFC 6266), e.g. "attachment", "inline".
    //! Stored lowercased - disposition-type is case-insensitive per the RFC.
    struct DispositionTypeHeader {
        std::string type{};

        bool operator==(const DispositionTypeHeader&) const = default;
    };
}

#include <Thoth/Http/NHeaders/Response/Headers/ContentDisposition.tpp>

namespace Thoth::Http::NHeaders {
    //! @brief Content-Disposition (RFC 6266): disposition-type plus parameters (commonly `filename`).
    //!
    //! @note `filename*` (RFC 8187 extended notation) is stored as a raw, still percent-encoded param
    //! under the key "filename*" - this doesn't decode the charset'lang'value encoding for you.
    using ContentDisposition = Parameterized<DispositionTypeHeader>;

    static_assert(Thoth::Utils::Serializable<DispositionTypeHeader>);
}
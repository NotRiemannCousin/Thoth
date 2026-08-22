#pragma once

namespace Thoth::Http::NHeaders {
    //! @brief Units in which a response advertises range support (RFC 9110 §14.3).
    //!
    //! Bytes` means byte ranges are supported and `None` explicitly advertises no range unit.
    enum class AcceptRanges {
        //! Range requests are not supported.
        None,
        //! Byte-range requests are supported.
        Bytes
    };
}

#include <Thoth/Http/NHeaders/Response/Headers/AcceptRanges.tpp>
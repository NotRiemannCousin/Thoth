#pragma once
#include <concepts>
#include <utility>

#include <Thoth/Http/_base/BodyIo/_base/_pch.hpp>
#include <Thoth/Http/Response/ResponseHead.hpp>
#include <Thoth/Http/Server/Definitions.hpp>

namespace Thoth::Http {

    //! @brief A Router decides what to do with an already-received exchange.
    //!
    //! `Resolve` answers with the lib's async function type carrying the `std::pair<ResponseHead, AnyBodyOut>` the
    //! Server writes, the Server normalizes every router at this single co_await. The body arrives type-erased
    //! (@ref AnyBody), so the you can use any body type you want (and even mix them!).
    template<class R, class Socket>
    concept RouterForConcept = requires(R router, Exchange<Socket>& exchange) {
            { router.Resolve(exchange) } -> std::same_as<exec::task<std::pair<ResponseHead, AnyBodyOut>>>;
        };
}

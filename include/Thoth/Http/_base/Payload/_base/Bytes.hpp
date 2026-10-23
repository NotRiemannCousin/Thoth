#pragma once
#include <cstddef>
#include <utility>
#include <vector>
#include <string>

#include <Thoth/Http/_base/Payload/_base.hpp>

namespace Thoth::Http {
    namespace details_ {
        template<class R>
        struct BytesIntoPayload {
            std::vector<std::byte> body;

            BytesIntoPayload(R&& body);

            operator IntoResponsePayload<std::vector<std::byte>>() &&;
            operator IntoRequestPayload<std::vector<std::byte>>() &&;
        };
    }

    template<std::ranges::range R>
        requires (!std::constructible_from<std::string, R>) &&
                 std::constructible_from<std::vector<std::byte>, std::from_range_t, R>
    struct IntoPayload<std::vector<std::byte>, R> : details_::BytesIntoPayload<R> {
        using details_::BytesIntoPayload<R>::BytesIntoPayload;
    };
}

#include <Thoth/Http/_base/Payload/_base/Bytes.tpp>

namespace Thoth::Http {
    static_assert(IntoRequestPayloadConcept<std::vector<std::byte>, std::vector<std::byte>>);
    static_assert(IntoResponsePayloadConcept<std::vector<std::byte>, std::vector<std::byte>>);
    static_assert(IntoRequestPayloadConcept<std::vector<std::byte>, std::span<std::byte>>);
    static_assert(IntoResponsePayloadConcept<std::vector<std::byte>, std::span<std::byte>>);
}
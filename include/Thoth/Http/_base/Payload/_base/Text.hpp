#pragma once
#include <string>
#include <utility>

#include <Thoth/Http/_base/Payload/_base.hpp>

namespace Thoth::Http {
    namespace details_ {
        template<class T>
        struct TextIntoPayload {
            std::string body;

            TextIntoPayload(T body);

            operator IntoResponsePayload<std::string>() &&;
            operator IntoRequestPayload<std::string>() &&;
        };
    }

    template<class T>
        requires std::constructible_from<std::string, T>
    struct IntoPayload<std::string, T> : details_::TextIntoPayload<T> {
        using details_::TextIntoPayload<T>::TextIntoPayload;
    };
}

#include <Thoth/Http/_base/Payload/_base/Text.tpp>

namespace Thoth::Http {
    static_assert(IntoRequestPayloadConcept<std::string, std::string>);
    static_assert(IntoResponsePayloadConcept<std::string, std::string>);
    static_assert(IntoRequestPayloadConcept<std::string, const char*>);
    static_assert(IntoResponsePayloadConcept<std::string, const char*>);
}
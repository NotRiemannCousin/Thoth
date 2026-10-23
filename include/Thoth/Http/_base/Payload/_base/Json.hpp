#pragma once
#include <string>

#include <Thoth/NJson/Json.hpp>
#include <Thoth/Http/_base/Payload/_base.hpp>

namespace Thoth::Http {
    namespace details_ {
        template<BodyConcept TargetBody, class T>
        struct JsonIntoPayload {
            T val;

            JsonIntoPayload(T body);

            operator IntoResponsePayload<TargetBody>() &&;
            operator IntoRequestPayload<TargetBody>() &&;
        };
    }

    template<BodyConcept TargetBody>
        requires std::ranges::output_range<TargetBody, char>
    struct IntoPayload<TargetBody, NJson::Json> : details_::JsonIntoPayload<TargetBody, NJson::Json> {
        using details_::JsonIntoPayload<TargetBody, NJson::Json>::JsonIntoPayload;
    };
    template<BodyConcept TargetBody>
        requires std::ranges::output_range<TargetBody, char>
    struct IntoPayload<TargetBody, NJson::JsonObject> : details_::JsonIntoPayload<TargetBody, NJson::JsonObject> {
        using details_::JsonIntoPayload<TargetBody, NJson::JsonObject>::JsonIntoPayload;
    };
}

#include <Thoth/Http/_base/Payload/_base/Json.tpp>

namespace Thoth::Http {
    static_assert(IntoRequestPayloadConcept<std::string, NJson::Json>);
    static_assert(IntoResponsePayloadConcept<std::string, NJson::Json>);
    static_assert(IntoRequestPayloadConcept<std::string, NJson::JsonObject>);
    static_assert(IntoResponsePayloadConcept<std::string, NJson::JsonObject>);
}
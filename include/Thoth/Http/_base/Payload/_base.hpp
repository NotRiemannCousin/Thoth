#pragma once
#include <expected>

#include <Thoth/ThothError.hpp>

#include <Thoth/Http/_base/BodyIo/_base.hpp>
#include <Thoth/Http/Methods/_base/MethodConcept.hpp>

namespace Thoth::Http {

    struct RequestHead;
    struct ResponseHead;

    template<MethodConcept Method, BodyConcept Body>
    struct Request;
    template<MethodConcept Method, BodyConcept Body>
    struct Response;

    template<MethodConcept Method, BodyConcept Body>
    struct RequestEntry;
    template<MethodConcept Method, BodyConcept Body>
    struct ResponseEntry;


#pragma region Payload Into

    template<BodyConcept TargetBody>
    using IntoRequestPayload = std::pair<RequestHead, TargetBody>;
    template<BodyConcept TargetBody>
    using IntoResponsePayload = std::pair<ResponseHead, TargetBody>;

    //! @brief CPO detailed in @ref RequestEntry and @ref ResponseEntry.
    //! @details You need to implement a constructor passing Source and a conversion to @ref IntoRequestPayload or
    //! @ref IntoResponsePayload (templates allowed).
    template<BodyConcept TargetBody, class Source>
    struct IntoPayload;


    //! @brief Normalized alias to IntoPayload.
    template<BodyConcept TargetBody, class Source>
    using IntoPayloadFor = IntoPayload<TargetBody, std::decay_t<Source>>;


    //! @brief A source that has an @ref IntoRequest specialization producing exactly @p Body.
    template<class TargetBody, class Source>
    concept IntoRequestPayloadConcept = BodyConcept<TargetBody> &&
                std::constructible_from<IntoPayloadFor<TargetBody, Source>, Source> &&
                std::convertible_to<IntoPayloadFor<TargetBody, Source>, IntoRequestPayload<TargetBody>>;

    //! @brief A source that has an @ref IntoResponse specialization producing exactly @p Body.
    template<class TargetBody, class Source>
    concept IntoResponsePayloadConcept = BodyConcept<TargetBody> &&
                std::constructible_from<IntoPayloadFor<TargetBody, Source>, Source> &&
                std::convertible_to<IntoPayloadFor<TargetBody, Source>, IntoResponsePayload<TargetBody>>;

    template<class TargetBody, class Source>
    concept IntoPayloadConcept = IntoRequestPayloadConcept<TargetBody, Source> || IntoResponsePayloadConcept<TargetBody, Source>;

    template<class TargetBody, class Source>
    concept SymetricPayloadConcept = IntoRequestPayloadConcept<TargetBody, Source> && IntoResponsePayloadConcept<TargetBody, Source>;

#pragma endregion

}
#pragma once
#include <type_traits>

#include <Thoth/Http/Methods/GetMethod.hpp>
#include <Thoth/Http/Methods/PostMethod.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/_base/Payload/_base/_pch.hpp>

namespace Thoth::Http {

    //! @brief Request's evil twin designed to help in conversions.
    //!
    //! Many libraries develop ways to convert from a "body like element" directly to a Request, even when the value
    //! isn't of the same type of the body itself. C++ doesn't have Rust's/Haskell's traits or prototypes, but we can
    //! achive this by using conversions and CPOs. We can't use conversion operators directly because we don't have the
    //! control of every target type (like `std::string` or `const char`), and isn't possible to make template
    //! conversion constructor for Request and Response while keeping these types aggregates. The solution is to merge
    //! both strategies in "evil twin versions" of @ref Request / @ref Response, these types acts like a bridge between
    //! your body-like and the request/response object.
    //!
    //! @dot
    //! digraph ConversionFlow {
    //!     rankdir=LR;
    //!     bgcolor="transparent";
    //!     node [shape=box, style="rounded,filled", fontname="Helvetica", fontsize=11, fillcolor="#2b303a", fontcolor="#ffffff", color="#4a90e2"];
    //!     edge [fontname="Helvetica", fontsize=9, color="#888888", fontcolor="#cccccc"];
    //!
    //!     str      [label="std::string"];
    //!     resEntry [label="ResponseEntry", URL="\ref ResponseEntry"];
    //!     res      [label="Response", URL="\ref Response"];
    //!
    //!     json     [label="NJson::Json"];
    //!     reqEntry [label="RequestEntry", URL="\ref RequestEntry"];
    //!     req      [label="Request", URL="\ref Request"];
    //!
    //!     // Fluxo Response
    //!     str -> resEntry [label="CPO via template\nconstructor"];
    //!     resEntry -> res [label="controlled conversion\noperator"];
    //!
    //!     // Fluxo Request
    //!     json -> reqEntry [label="CPO via template\nconstructor"];
    //!     reqEntry -> req  [label="controlled conversion\noperator"];
    //! }
    //! @enddot
    //!
    //! Of course, the CPO itself isn't tied to a specific MethodConcept, but RequestEntry and ResponseEntry are in
    //! order to mimic exatly the behavior of their counterpart, you can change `GetResponse<> Func(GetRequest<> req)`
    //! to `GetResponseEntry<> Func(GetRequest<> req)` with minimal changes.
    template<MethodConcept Method = GetMethod, BodyConcept Body = std::string>
    struct ResponseEntry {
        using MethodType   = Method;
        using BodyType     = Body;
        using ResponseType = Response<Method, Body>;

        //! @brief A default `Response` (200, default headers, empty body).
        ResponseEntry() = default;

        //! @brief Wraps a `Response` as it is, nothing is adapted.
        ResponseEntry(ResponseType response);

        //! @brief Builds the response from any source that has an @ref IntoResponse landing on `Body`, the
        //! intermediary consumes the source and adapts the head (`Content-Type`...).
        template<class Source>
            requires IntoResponsePayloadConcept<Body, Source>
        ResponseEntry(Source&& source);

        operator ResponseType() &&;

    private:
        ResponseType m_response{};
    };


    template<class T>    constexpr bool IsResponseEntry{ false };
    template<class... T> constexpr bool IsResponseEntry<ResponseEntry<T...>>{ true };

    template<class T>
    concept ResponseEntryConcept = IsResponseEntry<T>;

    using GetResponseEntry  = ResponseEntry<>;
    using PostResponseEntry = ResponseEntry<PostMethod>;

    using GetBinResponseEntry  = ResponseEntry<GetMethod, std::vector<std::byte>>;
    using PostBinResponseEntry = ResponseEntry<PostMethod, std::vector<std::byte>>;

    using GetFileResponseEntry  = ResponseEntry<GetMethod, FileBody>;
    using PostFileResponseEntry = ResponseEntry<PostMethod, FileBody>;
}

#include <Thoth/Http/Response/ResponseEntry.tpp>
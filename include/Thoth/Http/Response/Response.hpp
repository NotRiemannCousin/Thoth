#pragma once
#include <Thoth/Http/_base/BodyIo/_base/_pch.hpp>
#include <Thoth/Http/_base/Payload/_base.hpp>
#include <Thoth/Http/Response/ResponseHead.hpp>
#include <Thoth/Http/Methods/PostMethod.hpp>
#include <Thoth/Http/Methods/GetMethod.hpp>
#include <Thoth/NJson/Json.hpp>

namespace Thoth { struct ThothError; }

namespace Thoth::Http {
    template<MethodConcept Method = GetMethod, BodyConcept Body = std::string>
    struct Response : ResponseHead {
        using MethodType = Method;

        Body body;

        template<class = void>
            requires std::same_as<Body, std::string>
        [[nodiscard]] std::expected<NJson::Json, ThothError> AsJson() const;

        //! @brief Returns if the response is 2XX.
        [[nodiscard]] bool Successful() const;


        static std::expected<Response, ThothError> EnsureSuccess(Response&& response);

        //! @brief Monad friendly move of the body, discarding the rest of the response.
        //! Recommended check for Successful() before.
        [[nodiscard]] Body MoveBody() &&;
    };


    template<class T>    constexpr bool IsResponse{ false };
    template<class... T> constexpr bool IsResponse<Response<T...>>{ true };

    template<class T>
    concept ResponseConcept = IsResponse<T>;


    using GetResponse  = Response<>;
    using PostResponse = Response<PostMethod>;

    using GetBinResponse  = Response<GetMethod, std::vector<std::byte>>;
    using PostBinResponse = Response<PostMethod, std::vector<std::byte>>;

    using GetFileResponse  = Response<GetMethod, FileBody>;
    using PostFileResponse = Response<PostMethod, FileBody>;

    using GetEmptyResponse  = Response<GetMethod, EmptyBody>;
    using PostEmptyResponse = Response<PostMethod, EmptyBody>;
}



#include <Thoth/Http/Response/Response.tpp>

static_assert(Thoth::Http::IntoResponsePayloadConcept<std::string, Thoth::Http::GetResponse>);
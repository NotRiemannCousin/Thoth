#pragma once
#include <Thoth/Http/Response/ResponseHead.hpp>
#include <Thoth/Http/Methods/PostMethod.hpp>
#include <Thoth/Http/Methods/GetMethod.hpp>
#include <Thoth/Http/_base.hpp>
#include <Thoth/NJson/Json.hpp>
#include <Thoth/Dsa/FileOutputRange.hpp>

#include <Thoth/Http/NHeaders/Response/ResponseHeaders.hpp>


namespace Thoth {
    struct ThothError;
}

namespace Thoth::Http {

    template<WritableBodyConcept Body>
    auto GetInserterIterator(Body& body);


    template<MethodConcept Method = GetMethod, WritableBodyConcept Body = std::string>
    struct Response : ResponseHead {
        using MethodType = Method;

        Body body;

        template<class = void>
            requires std::same_as<Body, std::string>
        [[nodiscard]] std::expected<NJson::Json, ThothError> AsJson() const;

        //! @brief Returns if the response is 2XX.
        [[nodiscard]] bool Successful() const;


        static std::expected<Response, ThothError> EnsureSuccess(Response&& response);

        // template<>
        // static std::expected<Response, Response> SplitResult();

        //! @brief Monad friendly move of the body, discarding the rest of the response.
        //! Recommended check for Successful() before.
        [[nodiscard]] Body MoveBody() &&;
    };

    using GetResponse  = Response<>;
    using PostResponse = Response<PostMethod>;

    using GetBinResponse  = Response<GetMethod, std::vector<std::byte>>;
    using PostBinResponse = Response<PostMethod, std::vector<std::byte>>;

    using GetFileResponse  = Response<GetMethod, Dsa::TextFileOutputRange>;
    using PostFileResponse = Response<PostMethod, Dsa::TextFileOutputRange>;

    using GetFileBinResponse  = Response<GetMethod, Dsa::BinFileOutputRange>;
    using PostFileBinResponse = Response<PostMethod, Dsa::BinFileOutputRange>;
}



#include <Thoth/Http/Response/Response.tpp>


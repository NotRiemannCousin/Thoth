#pragma once
#include <Thoth/Http/Response/ResponseHead.hpp>
#include <Thoth/Http/Methods/PostMethod.hpp>
#include <Thoth/Http/Methods/GetMethod.hpp>
#include <Thoth/NJson/Json.hpp>
#include <Thoth/Dsa/FileOutputRange.hpp>

#include <Thoth/Http/NHeaders/Response/ResponseHeaders.hpp>


namespace Thoth::Http {
    struct RequestError;

    template<class T>
    concept ResponseBodyConcept =
            std::ranges::output_range<T, char> ||
            std::ranges::output_range<T, unsigned char> ||
            std::ranges::output_range<T, std::byte>;


    template<ResponseBodyConcept Body>
    auto GetInserterIterator(Body& body);


    template<MethodConcept Method = GetMethod, ResponseBodyConcept Body = std::string>
    struct Response {
        using MethodType = Method;

        VersionEnum version{};
        StatusCodeEnum status{};
        string statusMessage{};
        ResponseHeaders headers{};
        Body body;

        friend struct Client; // who construct it

        template<class = void>
            requires std::same_as<Body, string>
        [[nodiscard]] std::expected<NJson::Json, RequestError> AsJson() const;

        //! @brief Returns if the response is 2XX.
        [[nodiscard]] bool Successful() const;

        //! @brief Monad friendly move of the body, discarding the rest of the response.
        //! Recommended check for Successful() before.
        [[nodiscard]] Body MoveBody() &&;
    private:

        Response(VersionEnum version, StatusCodeEnum status,
                string&& statusMessage, Headers&& headers, Body&& body);

        Response(ResponseHead&& head, Body&& body);
    };

    using GetResponse  = Response<>;
    using PostResponse = Response<PostMethod>;

    using GetBinResponse  = Response<GetMethod, vector<std::byte>>;
    using PostBinResponse = Response<PostMethod, vector<std::byte>>;

    using GetFileResponse  = Response<GetMethod, Dsa::TextFileOutputRange>;
    using PostFileResponse = Response<PostMethod, Dsa::TextFileOutputRange>;

    using GetFileBinResponse  = Response<GetMethod, vector<std::byte>>;
    using PostFileBinResponse = Response<PostMethod, vector<std::byte>>;
}



#include <Thoth/Http/Response/Response.tpp>


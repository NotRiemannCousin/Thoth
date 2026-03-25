#pragma once
#include <Hermes/Socket/ClientSocket.hpp>

#include <Thoth/Http/Request/Url.hpp>
#include <Thoth/Http/Methods/GetMethod.hpp>
#include <Thoth/Http/Methods/PostMethod.hpp>

#include <Thoth/Http/NHeaders/Request/RequestHeaders.hpp>

namespace Thoth::Http {
	enum class VersionEnum : uint8_t {
		HTTP1_0,
		HTTP1_1,
		HTTP2,
		HTTP3,
	};

    //! @brief Exactly what you think it is.
    std::string_view VersionToString(VersionEnum version);



    template<class T>
    concept SizedRequestBodyConcept = std::ranges::input_range<T>
        && std::ranges::sized_range<T>
        && (
            std::same_as<std::ranges::range_value_t<T>, char> ||
            std::same_as<std::ranges::range_value_t<T>, unsigned char> ||
            std::same_as<std::ranges::range_value_t<T>, std::byte>
        );

    template<class T>
    concept ChunkedRequestBodyConcept = SizedRequestBodyConcept<std::ranges::range_value_t<T>>;

    template<class T>
    concept RequestBodyConcept = SizedRequestBodyConcept<T> || ChunkedRequestBodyConcept<T>;



	template<MethodConcept Method = GetMethod, RequestBodyConcept Body = std::string>
	struct Request {
		using MethodType = Method;

		Url url;
		Body body;
		VersionEnum version{ VersionEnum::HTTP1_1 };
		RequestHeaders headers{ Headers::DefaultHeaders() };

	    //! @brief Try parse to a URL before construct the Request.
		template<class T = string_view>
			requires Hermes::ByteLike<std::ranges::range_value_t<T>>
	                || (std::same_as<Body, std::string> && requires (T t) { { std::format("{}", t) }; })
		static std::expected<Request, RequestError> FromUrl(
			string_view url, T&& body = {}, Headers headers = Headers::DefaultHeaders());
	};

	using GetRequest  = Request<>;
	using PostRequest = Request<PostMethod>;

    using GetBinRequest  = Request<GetMethod, vector<std::byte>>;
    using PostBinRequest = Request<PostMethod, vector<std::byte>>;
}


#include <Thoth/Http/Request/Request.tpp>
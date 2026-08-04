#pragma once
#include <Hermes/Socket/Sync/ClientSocket.hpp>

#include <Thoth/Http/Request/RequestHead.hpp>
#include <Thoth/Http/_base.hpp>
#include <Thoth/Http/Url/Url.hpp>
#include <Thoth/Http/Methods/GetMethod.hpp>
#include <Thoth/Http/Methods/PostMethod.hpp>

#include <Thoth/Http/NHeaders/Request/RequestHeaders.hpp>

namespace Thoth::Http {
	template<MethodConcept Method = GetMethod, ReadableBodyConcept Body = std::string>
	struct Request : RequestHead {
		using MethodType = Method;

		Body body;

	    //! @brief Try parse to a URL before construct the Request.
		template<class T = std::string_view>
			requires Hermes::ByteLike<std::ranges::range_value_t<T>>
	                || (std::same_as<Body, std::string> && std::formattable<T, char>)
		static std::expected<Request, ExchangeError> FromUrl(
			std::string_view url, T&& body = {}, Headers headers = Headers::DefaultHeaders());
	};

	using GetRequest  = Request<>;
	using PostRequest = Request<PostMethod>;

    using GetBinRequest  = Request<GetMethod, std::vector<std::byte>>;
    using PostBinRequest = Request<PostMethod, std::vector<std::byte>>;
}


#include <Thoth/Http/Request/Request.tpp>
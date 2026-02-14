#pragma once
#include <Hermes/Socket/ClientSocket.hpp>

#include <Thoth/Http/Request/Url.hpp>
#include <Thoth/Http/Methods/GetMethod.hpp>
#include <Thoth/Http/Methods/PostMethod.hpp>

namespace Thoth::Http {
	enum class VersionEnum {
		HTTP1_0,
		HTTP1_1,
		HTTP2,
		HTTP3,
	};

    //! @brief Exactly what you think it is.
	std::string_view VersionToString(VersionEnum version);

	template<MethodConcept Method = GetMethod>
	struct Request {
		using MethodType = Method;

		Url url;
		string body{};
		VersionEnum version{ VersionEnum::HTTP1_1 };
		Headers headers{ Headers::DefaultHeaders() };

	    //! @brief Try parse to a URL before construct the Request.
		template<class T = string_view>
			requires requires (T t) { { std::format("{}", t) }; }
		static std::expected<Request, string> FromUrl(
			string_view url, T&& body = {}, Headers headers = Headers::DefaultHeaders());
	};

	using GetRequest  = Request<>;
	using PostRequest = Request<PostMethod>;
}


#include <Thoth/Http/Request/Request.tpp>
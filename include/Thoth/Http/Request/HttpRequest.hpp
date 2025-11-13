#pragma once
#include <Hermes/Socket/ClientSocket.hpp>

#include <Thoth/Http/Request/HttpUrl.hpp>
#include <Thoth/Http/HttpMethods/GetHttpMethod.hpp>

namespace Thoth::Http {
	enum class HttpVersion {
		HTTP1_0,
		HTTP1_1,
		HTTP2,
		HTTP3,
	};

	std::string_view HttpVersionToString(HttpVersion version);

	template<HttpMethodConcept Method>
	struct HttpRequest {
		HttpUrl url;
		string body{};
		HttpVersion version{ HttpVersion::HTTP1_1 };
		HttpHeaders headers{ HttpHeaders::DefaultHeaders() };

		template<class T>
			requires requires (T t) { { std::format("{}", t) }; }
		static std::optional<HttpRequest> FromUrl(string_view url, T&& body);
	};

	using GetRequest = HttpRequest<HttpGetMethod>;
};


#include <Thoth/Http/Request/HttpRequest.tpp>
#pragma once
#include <Hermes/Socket/ClientSocket.hpp>

#include <Thoth/Http/Request/Url.hpp>
#include <Thoth/Http/Methods/GetMethod.hpp>
#include <Thoth/Http/Methods/PostMethod.hpp>

namespace Thoth::Http {
	enum class Version {
		HTTP1_0,
		HTTP1_1,
		HTTP2,
		HTTP3,
	};

	std::string_view VersionToString(Version version);

	template<MethodConcept Method = GetMethod>
	struct Request {
		using MethodType = Method;

		Url url;
		string body{};
		Version version{ Version::HTTP1_1 };
		Headers headers{ Headers::DefaultHeaders() };

		template<class T = string_view>
			requires requires (T t) { { std::format("{}", t) }; }
		static std::optional<Request> FromUrl(string_view url, T&& body = {});
	};

	using GetRequest  = Request<>;
	using PostRequest = Request<PostMethod>;
}


#include <Thoth/Http/Request/Request.tpp>
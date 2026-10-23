#pragma once
#include <Hermes/Socket/Sync/ClientSocket.hpp>

#include <Thoth/Http/_base/BodyIo/_base/_pch.hpp>
#include <Thoth/Http/_base/Payload/_base.hpp>
#include <Thoth/Http/Request/RequestHead.hpp>
#include <Thoth/Http/Methods/GetMethod.hpp>
#include <Thoth/Http/Methods/PostMethod.hpp>
#include <Thoth/NJson/Json.hpp>

namespace Thoth::Http {
	template<MethodConcept Method = GetMethod, BodyConcept Body = std::string>
	struct Request : RequestHead {
		using MethodType = Method;

		Body body;


		template<class = void>
			requires std::same_as<Body, std::string>
		[[nodiscard]] std::expected<NJson::Json, ThothError> AsJson() const;

	    //! @brief Try parse to a URL before construct the Request.
		template<class T = std::string_view>
			requires Hermes::ByteLike<std::ranges::range_value_t<T>> && OutgoingBodyConcept<Body>
	                || (std::same_as<Body, std::string> && std::formattable<T, char>)
		static std::expected<Request, ThothError> FromUrl(
			std::string_view url, T&& body = {}, Headers headers = Headers::DefaultHeaders());
	};


	template<class T>	 constexpr bool IsRequest{ false };
	template<class... T> constexpr bool IsRequest<Request<T...>>{ true };

	template<class T>
	concept RequestConcept = IsRequest<T>;

	using GetRequest  = Request<>;
	using PostRequest = Request<PostMethod>;

    using GetBinRequest  = Request<GetMethod, std::vector<std::byte>>;
    using PostBinRequest = Request<PostMethod, std::vector<std::byte>>;

    using GetEmptyRequest  = Request<GetMethod, EmptyBody>;
    using PostEmptyRequest = Request<PostMethod, EmptyBody>;

}


#include <Thoth/Http/Request/Request.tpp>

static_assert(Thoth::Http::IntoRequestPayloadConcept<std::string, Thoth::Http::GetRequest>);
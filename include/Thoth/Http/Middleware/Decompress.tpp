#pragma once
#include <Thoth/Http/Middleware/Decompress.hpp>

namespace Thoth::Http {
    namespace Details_ {
        // TODO: Improve return type
        std::expected<std::string, ThothError> InflateGzipOrDeflate(std::span<char> compressed);

        template<std::ranges::contiguous_range R>
            requires Hermes::ByteLike<std::ranges::range_value_t<R>>
        std::expected<std::string, ThothError> RangeInflateGzipOrDeflate(R&& compressed) {
            const std::span<char> range{
                reinterpret_cast<char*>(const_cast<std::remove_cv_t<std::remove_pointer_t<decltype(
                    std::ranges::data(compressed))>>*>(std::ranges::data(compressed))),
                std::ranges::size(compressed)
            };

            return InflateGzipOrDeflate(range);;
        }
    }


    template<class Next>
    auto Decompress(Next next) {
        return [next = std::move(next)]<MethodConcept Method, BodyConcept Body>
            (Request<Method, Body> request) mutable -> Client::ExpResponse<Method, Body>
            requires HandlerConcept<Next, Method, Body> && std::ranges::contiguous_range<Body>
        {
            namespace rg = std::ranges;
            namespace vs = std::views;

            auto response{ next(std::move(request)) };
            if (!response) return response;

            auto codings{ response->headers.ContentEncoding().GetAsOpt() };
            if (!codings || codings->empty()) return response;

            for (auto encoding : *codings | vs::reverse) {
                switch (encoding) {
                    case NHeaders::ContentEncodingEnum::Gzip:
                    case NHeaders::ContentEncodingEnum::Deflate: {
                        auto decoded{ Details_::RangeInflateGzipOrDeflate(response->body) };
                        if (!decoded) return std::unexpected{ decoded.error() };
                        response->body = std::move(*decoded);
                        break;
                    }
                    default:
                        return ThothUnex{ GenericError{
                            "Decompress: unsupported Content-Encoding (only gzip/deflate linked)" } };
                }
            }
            response->headers.Remove("content-encoding");
            response->headers.Remove("content-length");
            return response;
        };
    }
}
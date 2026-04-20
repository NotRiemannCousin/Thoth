#pragma once
#include <Thoth/Http/Response/ResponseHead.hpp>

namespace Thoth::Dsa {
    template<Hermes::ByteLike T>
    constexpr int FileOutputRange<T>::Mode() {
        return std::same_as<T, std::byte>
                ? std::ios::out | std::ios::binary
                : std::ios::out;
    }

    template<Hermes::ByteLike T>
    constexpr auto FileOutputRange<T>::H_AsBody(const FileBuilderParams& params) {
        return [&](const Http::ResponseHead& head) -> BodyType {
            using Exp = std::expected<std::monostate, Http::RequestError>;

            auto s_checkLen = [&]() -> Exp {
                const auto length{ head.headers.ContentLength().Get() };

                if (!length)
                    return std::unexpected{ Http::RequestError{ Http::RequestBuildErrorEnum::InvalidHeaders } };
                if (params.maxSize < *length)
                    return std::unexpected{ Http::RequestError{ Http::GenericError{ "response size is bigger than the permitted" } } };
                return {};
            };

            auto s_checkType = [&](auto) -> Exp {
                if (!params.acceptedTypes) return {};

                const auto type{ head.headers.ContentType().GetWithDefault(Http::NHeaders::MimeTypes::appOctetStream) };
                if (!type) return std::unexpected{ Http::RequestError{ Http::RequestBuildErrorEnum::InvalidHeaders } };

                if (!std::ranges::contains(*params.acceptedTypes, *type))
                    return std::unexpected{ Http::RequestError{ Http::GenericError{ "type not accepted" } } };

                return {};
            };

            auto s_happyPath = [&](auto) -> std::expected<FileOutputRange, Http::RequestError> {
                std::error_code ec;
                std::filesystem::create_directories(params.path.parent_path(), ec);
                if (ec) return std::unexpected{ Http::RequestError{ Http::GenericError{ "Unable to create file path" } } };

                return FileOutputRange{ params.path, params.mode };
            };

            return s_checkLen().and_then(s_checkType).and_then(s_happyPath);
        };
    }

    template<Hermes::ByteLike T>
    constexpr auto FileOutputRange<T>::H_AsBody(FileBuilderParams&& params) {
        return [params = std::move(params)](const Http::ResponseHead& head) mutable -> BodyType {
            using Exp = std::expected<std::monostate, Http::RequestError>;

            auto s_checkLen = [&]() -> Exp {
                const auto length{ head.headers.ContentLength().Get() };

                if (!length)
                    return std::unexpected{ Http::RequestError{ Http::RequestBuildErrorEnum::InvalidHeaders } };
                if (params.maxSize < *length)
                    return std::unexpected{ Http::RequestError{ Http::GenericError{ "response size is bigger than the permitted" } } };
                return {};
            };

            auto s_checkType = [&](auto) -> Exp {
                if (!params.acceptedTypes) return {};

                const auto type{ head.headers.ContentType().GetWithDefault(Http::NHeaders::MimeTypes::appOctetStream) };
                if (!type) return std::unexpected{ Http::RequestError{ Http::RequestBuildErrorEnum::InvalidHeaders } };

                if (!std::ranges::contains(*params.acceptedTypes, *type))
                    return std::unexpected{ Http::RequestError{ Http::GenericError{ "type not accepted" } } };

                return {};
            };

            auto s_happyPath = [&](auto) -> std::expected<FileOutputRange, Http::RequestError> {
                std::error_code ec;
                std::filesystem::create_directories(params.path.parent_path(), ec);
                if (ec) return std::unexpected{ Http::RequestError{ Http::GenericError{ "Unable to create file path" } } };

                return FileOutputRange{ std::move(params.path), params.mode };
            };

            return s_checkLen().and_then(s_checkType).and_then(s_happyPath);
        };
    }

    template<Hermes::ByteLike T>
    FileOutputRange<T>::FileOutputRange(const std::filesystem::path& path, const int mode) : _outStream{ path, Mode() | mode } {}

    template<Hermes::ByteLike T>
    FileOutputRange<T>::FileOutputRange(std::filesystem::path&& path, const int mode) : _outStream{ std::move(path), Mode() | mode } {}

    template<Hermes::ByteLike T>
    FileOutputRange<T>::FileOutputRange(std::basic_ofstream<T> &&file) : _outStream{ std::move(file) } {}

    template<Hermes::ByteLike T>
    FileOutputRange<T>::FileOutputRange(FileOutputRange&& other) noexcept : _outStream{ std::move(other._outStream) } {}

    template<Hermes::ByteLike T>
    FileOutputRange<T> & FileOutputRange<T>::operator=(FileOutputRange &&other) noexcept {
        _outStream = std::move(other._outStream);
        return *this;
    }

    template<Hermes::ByteLike T>
    std::ostream_iterator<T, T> FileOutputRange<T>::begin() {
        return std::ostream_iterator<T, T>(_outStream);
    }

    template<Hermes::ByteLike T>
    std::unreachable_sentinel_t FileOutputRange<T>::end() {
        return std::unreachable_sentinel;
    }
}

template<Hermes::ByteLike T>
inline constexpr bool std::ranges::enable_borrowed_range<Thoth::Dsa::FileOutputRange<T>> = true;


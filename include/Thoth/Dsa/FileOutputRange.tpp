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
            auto checkLen{ [&]() -> ThothResultOper {
                const auto length{ head.headers.ContentLength().Get() };

                if (!length)
                    return ThothUnex{{ Http::MessageParseErrorEnum::InvalidHeaders }};
                if (params.maxSize < *length)
                    return ThothUnex{{ GenericError{ "response size is bigger than the permitted" } }};
                return {};
            } };

            auto checkType{ [&](auto) -> ThothResultOper {
                if (!params.acceptedTypes) return {};

                const auto type{ head.headers.ContentType().GetWithDefault(Http::NHeaders::MimeTypes::appOctetStream) };
                if (!type) return ThothUnex{{ Http::MessageParseErrorEnum::InvalidHeaders }};

                if (!std::ranges::contains(*params.acceptedTypes, *type))
                    return ThothUnex{{ GenericError{ "type not accepted" } }};

                return {};
            } };

            auto happyPath{ [&](auto) -> std::expected<FileOutputRange, ThothError> {
                std::error_code ec;
                std::filesystem::create_directories(params.path.parent_path(), ec);
                if (ec) return ThothUnex{{ GenericError{ "Unable to create file path" } }};

                return FileOutputRange{ params.path, params.mode };
            } };

            return checkLen().and_then(checkType).and_then(happyPath);
        };
    }

    template<Hermes::ByteLike T>
    constexpr auto FileOutputRange<T>::H_AsBody(FileBuilderParams&& params) {
        return [params = std::move(params)](const Http::ResponseHead& head) -> BodyType {
            using Exp = std::expected<std::monostate, ThothError>;

            auto checkLen{ [&]() -> Exp {
                const auto length{ head.headers.ContentLength().Get() };

                if (!length)
                    return ThothUnex{{ Http::MessageParseErrorEnum::InvalidHeaders }};
                if (params.maxSize < *length)
                    return ThothUnex{{ GenericError{ "response size is bigger than the permitted" } }};
                return {};
            } };

            auto checkType{ [&](auto) -> Exp {
                if (!params.acceptedTypes) return {};

                const auto type{ head.headers.ContentType().GetWithDefault(Http::NHeaders::MimeTypes::appOctetStream) };
                if (!type) return ThothUnex{{ Http::MessageParseErrorEnum::InvalidHeaders }};

                if (!std::ranges::contains(*params.acceptedTypes, *type))
                    return ThothUnex{{ GenericError{ "type not accepted" } }};

                return {};
            } };

            auto happyPath{ [&](auto) -> std::expected<FileOutputRange, ThothError> {
                std::error_code ec;
                std::filesystem::create_directories(params.path.parent_path(), ec);
                if (ec) return ThothUnex{{ GenericError{ "Unable to create file path" } }};

                return FileOutputRange{ std::move(params.path), params.mode };
            } };

            return checkLen().and_then(checkType).and_then(happyPath);
        };
    }

    template<Hermes::ByteLike T>
    FileOutputRange<T>::FileOutputRange(const std::filesystem::path& path, const int mode)
        : m_outStream{ path, static_cast<std::ios_base::openmode>(Mode() | mode) } {}

    template<Hermes::ByteLike T>
    FileOutputRange<T>::FileOutputRange(std::filesystem::path&& path, const int mode)
        : m_outStream{ std::move(path), static_cast<std::ios_base::openmode>(Mode() | mode) } {}

    template<Hermes::ByteLike T>
    FileOutputRange<T>::FileOutputRange(std::basic_ofstream<T> &&file) : m_outStream{ std::move(file) } {}

    template<Hermes::ByteLike T>
    FileOutputRange<T>::FileOutputRange(FileOutputRange&& other) noexcept : m_outStream{ std::move(other.m_outStream) } {}

    template<Hermes::ByteLike T>
    FileOutputRange<T> & FileOutputRange<T>::operator=(FileOutputRange &&other) noexcept {
        m_outStream = std::move(other.m_outStream);
        return *this;
    }

    template<Hermes::ByteLike T>
    FileOutputIterator<T> FileOutputRange<T>::begin() {
        return FileOutputIterator<T>{ &m_outStream };
    }

    template<Hermes::ByteLike T>
    std::unreachable_sentinel_t FileOutputRange<T>::end() {
        return std::unreachable_sentinel;
    }
}

template<Hermes::ByteLike T>
inline constexpr bool std::ranges::enable_borrowed_range<Thoth::Dsa::FileOutputRange<T>> = true;


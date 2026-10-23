#pragma once

#include <algorithm>
#include <filesystem>
#include <utility>

#include <Thoth/Http/Response/ResponseHead.hpp>

namespace Thoth::Http {

    constexpr auto FileBody::H_AsBody(const FileBuilderParams& params) {
        return [&params](const ResponseHead& head) -> BodyType {
            auto checkLen{ [&]() -> ThothResultOper {
                const auto length{ head.headers.ContentLength().Get() };

                if (!length)
                    return ThothUnex{{ MessageParseErrorEnum::InvalidHeaders }};
                if (params.maxSize < *length)
                    return ThothUnex{{ GenericError{ "response size is bigger than the permitted" } }};
                return {};
            } };

            auto checkType{ [&](auto) -> ThothResultOper {
                if (!params.acceptedTypes) return {};

                const auto type{ head.headers.ContentType().GetWithDefault(NHeaders::MimeTypes::appOctetStream) };
                if (!type) return ThothUnex{{ MessageParseErrorEnum::InvalidHeaders }};

                if (!std::ranges::contains(*params.acceptedTypes, *type))
                    return ThothUnex{{ GenericError{ "type not accepted" } }};

                return {};
            } };

            auto happyPath{ [&](auto) -> BodyType {
                std::error_code ec;
                std::filesystem::create_directories(params.path.parent_path(), ec);
                if (ec) return ThothUnex{{ GenericError{ "Unable to create file path" } }};

                return FileBody{ params.path, params.mode };
            } };

            return checkLen().and_then(checkType).and_then(happyPath);
        };
    }

    constexpr auto FileBody::H_AsBody(FileBuilderParams&& params) {
        return [params = std::move(params)](const ResponseHead& head) -> BodyType {
            auto checkLen{ [&]() -> ThothResultOper {
                const auto length{ head.headers.ContentLength().Get() };

                if (!length)
                    return ThothUnex{{ MessageParseErrorEnum::InvalidHeaders }};
                if (params.maxSize < *length)
                    return ThothUnex{{ GenericError{ "response size is bigger than the permitted" } }};
                return {};
            } };

            auto checkType{ [&](auto) -> ThothResultOper {
                if (!params.acceptedTypes) return {};

                const auto type{ head.headers.ContentType().GetWithDefault(NHeaders::MimeTypes::appOctetStream) };
                if (!type) return ThothUnex{{ MessageParseErrorEnum::InvalidHeaders }};

                if (!std::ranges::contains(*params.acceptedTypes, *type))
                    return ThothUnex{{ GenericError{ "type not accepted" } }};

                return {};
            } };

            auto happyPath{ [&](auto) -> BodyType {
                std::error_code ec;
                std::filesystem::create_directories(params.path.parent_path(), ec);
                if (ec) return ThothUnex{{ GenericError{ "Unable to create file path" } }};

                return FileBody{ std::move(params.path), params.mode };
            } };

            return checkLen().and_then(checkType).and_then(happyPath);
        };
    }

    inline bool BodyIo<FileBody>::OpenOut(const FileBody& body)  {
        if (m_outOpened)
            return m_out.is_open();

        m_out.open(body.path,
                   static_cast<std::ios_base::openmode>(std::ios::out | std::ios::binary | body.mode));
        m_outOpened = true;
        return m_out.is_open();
    }

    inline std::optional<std::uint64_t> BodyIo<FileBody>::Size(const FileBody& body) const {
        std::error_code ec;
        const auto      size{ std::filesystem::file_size(body.path, ec) };
        if (ec)
            return std::nullopt;

        return static_cast<std::uint64_t>(size);
    }

    inline ReadResult BodyIo<FileBody>::Read(const FileBody& body) {
        if (!m_inOpened) {
            m_in.open(body.path, std::ios::in | std::ios::binary);
            m_inOpened = true;
        }

        if (!m_in.is_open())
            return std::unexpected{ ThothError{ GenericError{ "Unable to open file body" } } };

        if (m_bounce.size() < k_bounceBytes)
            m_bounce.resize(k_bounceBytes);

        m_in.read(reinterpret_cast<char*>(m_bounce.data()), static_cast<std::streamsize>(m_bounce.size()));

        const auto got{ m_in.gcount() };
        if (got <= 0)
            return std::optional<ByteView>{}; // clean EOF, end of body

        return ByteView{ m_bounce.data(), static_cast<std::size_t>(got) };
    }

    inline WriteResult BodyIo<FileBody>::Write(FileBody& body, std::span<std::byte> window) {
        if (!OpenOut(body))
            return { window.size(), std::unexpected{ ThothError{ GenericError{ "Unable to open file body" } } } };

        m_out.write(reinterpret_cast<const char*>(window.data()), static_cast<std::streamsize>(window.size()));
        if (!m_out)
            return { window.size(),
                     std::unexpected{ ThothError{ GenericError{ "Unable to write file body" } } } };

        return { window.size(), {} };
    }

    inline FinishResult BodyIo<FileBody>::FinishWrite(FileBody& body) {
        if (!OpenOut(body) || !m_out.flush())
            return std::unexpected{ ThothError{ GenericError{ "Unable to flush file body" } } };

        return {};
    }
}

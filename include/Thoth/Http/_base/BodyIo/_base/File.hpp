#pragma once
#include <filesystem>
#include <fstream>
#include <limits>
#include <optional>
#include <span>
#include <vector>

#include <Thoth/Http/NHeaders/Headers/MimeType.hpp>
#include <Thoth/Http/_base/BodyIo/_base.hpp>

namespace Thoth::Http {

    //! @brief Builder knobs of the download factory (@ref FileBody::H_AsBody): where to write, which content
    //! types to accept and the response size ceiling. The checks run against the response head, before any
    //! body byte is consumed, so a refused download never drains the stream.
    struct FileBuilderParams {
        std::filesystem::path path;
        std::optional<std::vector<NHeaders::MimeType>> acceptedTypes{};
        int maxSize{ std::numeric_limits<int>::max() };
        int mode{};
    };

    //! @brief A file on disk as a body, symmetric. The outgoing side serves it: `Size` comes from the filesystem
    //! metadata (no byte is read) and Read hands out slices through an internal bounce buffer, so a multi-gigabyte
    //! upload never gets materialized. The incoming side downloads into it: Write appends whole windows to the
    //! file and FinishWrite flushes (the RAII close on destruction stays as the failure-path backstop, where
    //! flushing would be wrong anyway). The stream is always binary, bytes are bytes and no EOL magic is welcome
    //! on the wire.
    struct FileBody {
        //! @brief The factory result, the alias every @ref BodyFactoryConcept consumer looks for.
        using BodyType = std::expected<FileBody, ThothError>;

        //! @brief The download factory (a BodyFactory): validates the response head (declared length against
        //! @ref FileBuilderParams::maxSize, content type against @ref FileBuilderParams::acceptedTypes), creates
        //! the destination directory and hands back the body. Runs before any body byte is consumed.
        static constexpr auto H_AsBody(const FileBuilderParams& params);
        static constexpr auto H_AsBody(FileBuilderParams&& params);

        std::filesystem::path path;
        int                   mode{};
    };

    template<>
    struct BodyIo<FileBody> {
        //! @brief Slice ceiling of the outgoing side / window buffer of the incoming one. One buffer, reused
        //! between slices, allocated on the first use.
        static constexpr std::size_t k_bounceBytes{ 8 * 1024 };

    private:
        mutable std::ifstream  m_in{};
        std::ofstream          m_out{};
        std::vector<std::byte> m_bounce{};
        bool                   m_inOpened{};
        bool                   m_outOpened{};

        //! @brief Opens the incoming stream on the first use (a fresh BodyIo is a fresh exchange, which is also
        //! the retry story). @p truncate honours the open mode the body carries.
        bool OpenOut(const FileBody& body);

    public:
        //! @brief File size from the filesystem. `nullopt` when the file cannot be stat'ed, which makes the transport
        //! fall back to chunked framing while the real open error surfaces at Read.
        [[nodiscard]] std::optional<std::uint64_t> Size(const FileBody& body) const;

        //! @brief Opens the file on the first Read and keeps it open between slices.
        ReadResult Read(const FileBody& body);

        //! @brief Appends the whole window to the file. A file sink consumes everything or fails, there is no
        //! meaningful partial consume on an ofstream.
        WriteResult Write(FileBody& body, std::span<std::byte> window);

        //! @brief Flushes the incoming stream. A download that never received a byte still materializes the
        //! file here, which is what the eager open of the old output range used to do.
        FinishResult FinishWrite(FileBody& body);
    };
}

#include <Thoth/Http/_base/BodyIo/_base/File.tpp>


static_assert(Thoth::Http::SymetricBodyConcept<Thoth::Http::FileBody>);
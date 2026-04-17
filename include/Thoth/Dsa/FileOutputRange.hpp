#pragma once
#include <Hermes/Socket/_base/_base.hpp>
#include <filesystem>
#include <fstream>

namespace Thoth::Dsa {
    struct FileBuilderParams {
        std::filesystem::path path;
        std::optional<std::vector<Http::NHeaders::MimeType>> acceptedTypes{};
        int maxSize{ INT_MAX };
        int mode{};
    };

    template <Hermes::ByteLike T = char>
    struct FileOutputRange {

        using value_type = T;
        using BodyType   = std::expected<FileOutputRange, Http::RequestError>;

        static constexpr auto H_AsBody(const FileBuilderParams& params);
        static constexpr auto H_AsBody(FileBuilderParams&& params);

        static constexpr int Mode();

        // change later to ensure that path is a regular file
        explicit FileOutputRange(const std::filesystem::path& path, int mode = 0);
        explicit FileOutputRange(std::filesystem::path&& path, int mode = 0);

        explicit FileOutputRange(std::basic_ofstream<T>&& file);

        FileOutputRange(FileOutputRange&& other) noexcept;
        FileOutputRange(const FileOutputRange& other) = delete;

        FileOutputRange& operator=(FileOutputRange&& other) noexcept;
        FileOutputRange& operator=(const FileOutputRange& other) = delete;

        constexpr bool operator==(const FileOutputRange& other) const = default;

        [[nodiscard]] std::ostream_iterator<T, T> begin();

        [[nodiscard]] static std::unreachable_sentinel_t end();

    private:
        std::basic_ofstream<T> _outStream;
    };

    using TextFileOutputRange = FileOutputRange<>;
    using BinFileOutputRange  = FileOutputRange<std::byte>;


    static_assert(std::ranges::range<TextFileOutputRange>);
}

#include <Thoth/Dsa/FileOutputRange.tpp>

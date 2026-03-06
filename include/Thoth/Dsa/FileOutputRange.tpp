#pragma once

namespace Thoth::Dsa {
    template<Hermes::ByteLike T>
    constexpr int FileOutputRange<T>::Mode() {
        return std::same_as<T, std::byte>
                ? std::ios::out | std::ios::binary
                : std::ios::out;
    }

    template<Hermes::ByteLike T>
    constexpr auto FileOutputRange<T>::H_AsBody(const std::filesystem::path& path, const int mode) {
        return [&]() -> BodyType {
            if (!is_regular_file(path))
                return std::unexpected{ Http::RequestError{ Http::GenericError{ std::format("'{}' is not a file", path.string()) } } };
            create_directories(path.parent_path());
            return FileOutputRange{ path, mode };
        };
    }

    template<Hermes::ByteLike T>
    constexpr auto FileOutputRange<T>::H_AsBody(std::filesystem::path&& path, const int mode) {
        return [path = std::move(path), mode = mode]() -> BodyType {
            if (!is_regular_file(path))
                return std::unexpected{ Http::RequestError{ Http::GenericError{ std::format("'{}' is not a file", path.string()) } } };
            create_directories(path.parent_path());
            return FileOutputRange{ std::move(path), mode };
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


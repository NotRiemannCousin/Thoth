#include <Thoth/Http/Middleware/Decompress.hpp>
#include <zlib.h>
#include <array>

namespace Thoth::Http::details_ {
    std::expected<std::string, ThothError> InflateGzipOrDeflate(std::span<char> compressed) {
        z_stream stream{};
        if (inflateInit2(&stream, 32 + MAX_WBITS) != Z_OK)
            return ThothUnex{ GenericError{ "zlib: inflateInit2 failed" } };

        struct Guard { z_stream* s; ~Guard() { inflateEnd(s); } } guard{ &stream };

        std::string out;
        std::array<char, 16 * 1024> buffer{};
        stream.next_in  = reinterpret_cast<Bytef*>(compressed.data());
        stream.avail_in = static_cast<uInt>(compressed.size());

        int ret{};
        do {
            stream.next_out  = reinterpret_cast<Bytef*>(buffer.data());
            stream.avail_out = static_cast<uInt>(buffer.size());
            ret = inflate(&stream, Z_NO_FLUSH);
            if (ret != Z_OK && ret != Z_STREAM_END)
                return ThothUnex{ GenericError{ "zlib: inflate failed" } };
            out.append(buffer.data(), buffer.size() - stream.avail_out);
        } while (ret != Z_STREAM_END && stream.avail_in > 0);

        return out;
    }
}
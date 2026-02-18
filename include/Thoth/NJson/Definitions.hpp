#pragma once
#include <variant>
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <expected>
#include <concepts>
#include <span>

#include <Thoth/Dsa/Cow.hpp>
#include <Thoth/NJson/StringRef.hpp>


namespace Thoth::Http {
    struct RequestError;
}

namespace Thoth::NJson {
        struct JsonObject;
    struct Json;


    using Null   = std::monostate;                   // null
    using String = Dsa::Cow<StringRef, std::string>; // string
    using Number = long double;                      // number
    using Bool   = bool;                             // bool
    using Object = std::unique_ptr<JsonObject>;      // {Object}
    using Array  = std::vector<Json>;                // [Array]

    namespace Details {
        struct BufferInfo {
            std::string_view bufferView;
            std::shared_ptr<std::string> buffer;
        };

        static bool ReadString(std::string_view& input, auto& val, const BufferInfo& info);
        static bool ReadNumber(std::string_view& input, auto& val);
        static bool ReadObject(std::string_view& input, auto& val, const BufferInfo& info);
        static bool ReadBool  (std::string_view& input, auto& val);
        static bool ReadNull  (std::string_view& input, auto& val);
        static bool ReadArray (std::string_view& input, auto& val, const BufferInfo& info);
    }


#pragma region Wappers for std::optional and std::expected
    using RefValWrapper = Json*;
    using CRefValWrapper = const Json*;

    using OptRefValWrapper = std::optional<RefValWrapper>;
    using OptCRefValWrapper = std::optional<CRefValWrapper>;

    using ExpRefValWrapper = std::expected<RefValWrapper, Http::RequestError>;
    using ExpCRefValWrapper = std::expected<CRefValWrapper, Http::RequestError>;

    using ValWrapper = Json;
    using CValWrapper = const Json;

    using OptValWrapper = std::optional<ValWrapper>;
    using OptCValWrapper = std::optional<CValWrapper>;

    using ExpValWrapper = std::expected<ValWrapper, Http::RequestError>;
    using ExpCValWrapper = std::expected<CValWrapper, Http::RequestError>;
#pragma endregion



    using JsonObjKey    = std::string;
    using JsonObjKeyRef = std::string_view;

    using Key  = std::variant<int, JsonObjKey>;
    using Keys = std::span<const Key>;


    template<class ...T>
    requires ((std::unsigned_integral<T> || std::convertible_to<T, std::string_view>) &&...)
    auto MakeKeys(const T&... keys) { return std::array<Key, sizeof...(T)>{ (keys, ...) }; }
}
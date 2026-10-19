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
#include <Thoth/NJson/Number.hpp>
#include <Thoth/NJson/StringRef.hpp>
#include <Thoth/NJson/StringOwn.hpp>


namespace Thoth {
    struct ThothError;
}

namespace Thoth::NJson {
        struct JsonObject;
    struct Json;

    using Null   = std::monostate;                 // null
    using String = Dsa::Cow<StringRef, StringOwn>; // "string"
    using Number = Number;                         // number
    using Bool   = bool;                           // bool
    using Object = std::unique_ptr<JsonObject>;    // {Object}
    using Array  = std::vector<Json>;              // [Array]

    namespace details_ {
        struct BufferInfo {
            std::string_view bufferView;
            std::shared_ptr<const char[]> buffer;
        };

        bool ReadString(std::string_view& input, auto& val, const BufferInfo& info);
        bool ReadNumber(std::string_view& input, auto& val);
        bool ReadObject(std::string_view& input, auto& val, const BufferInfo& info);
        bool ReadBool  (std::string_view& input, auto& val);
        bool ReadNull  (std::string_view& input, auto& val);
        bool ReadArray (std::string_view& input, auto& val, const BufferInfo& info);
    }


#pragma region Wappers for std::optional and std::expected
    using RefValWrapper = Json*;
    using CRefValWrapper = const Json*;

    using OptRefValWrapper = std::optional<RefValWrapper>;
    using OptCRefValWrapper = std::optional<CRefValWrapper>;

    using ExpRefValWrapper = std::expected<RefValWrapper, ThothError>;
    using ExpCRefValWrapper = std::expected<CRefValWrapper, ThothError>;

    using ValWrapper = Json;
    using CValWrapper = const Json;

    using OptValWrapper = std::optional<ValWrapper>;
    using OptCValWrapper = std::optional<CValWrapper>;

    using ExpValWrapper = std::expected<ValWrapper, ThothError>;
    using ExpCValWrapper = std::expected<CValWrapper, ThothError>;
#pragma endregion



    using JsonObjKey    = std::string;
    using JsonObjKeyRef = std::string_view;
}
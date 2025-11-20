#pragma once
#include <variant>
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <concepts>
#include <span>

#include <Thoth/Dsa/Cow.hpp>
#include <Thoth/NJson/StringRef.hpp>

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
    using RefValWrapper = std::reference_wrapper<Json>;
    using CRefValWrapper = std::reference_wrapper<const Json>;

    using OptRefValWrapper = std::optional<RefValWrapper>;
    using OptCRefValWrapper = std::optional<CRefValWrapper>;

    using RefValWrapperOrNull = RefValWrapper;
    using CRefValWrapperOrNull = CRefValWrapper;

    using ValWrapper = Json;
    using CValWrapper = const Json;

    using OptValWrapper = std::optional<ValWrapper>;
    using OptCValWrapper = std::optional<CValWrapper>;

    using ValWrapperOrNull = ValWrapper;
    using CValWrapperOrNull = CValWrapper;
#pragma endregion



    using JsonObjKey    = std::string;
    using JsonObjKeyRef = std::string_view;

    using Key = std::variant<int, JsonObjKey>;


    template<class ...T>
    requires ((std::unsigned_integral<T> || std::convertible_to<T, std::string_view>) &&...)
    auto MakeKeys(const T&... keys) { return std::array<Key, sizeof...(T)>{ (keys, ...) }; }

    struct Json{
        using Value = std::variant<Null, String, Number, Bool, Object, Array>;

        // NOLINTBEGIN(*)
        Json();
        Json(const JsonObject& child);
        Json(JsonObject&& child);

        Json(Value&& newValue) noexcept;
        Json(const Value& newValue);
        Json(Json&& other) noexcept;
        Json(const Json& other);


        Json& operator=(const JsonObject& other);
        Json& operator=(JsonObject&& other);

        Json& operator=(Value&& newValue) noexcept;
        Json& operator=(const Value& newValue);
        Json& operator=(Json&& other) noexcept;
        Json& operator=(const Json& other);


        operator Value&();
        [[nodiscard]] operator const Value&() const;
        // NOLINTEND(*)

        template<class T>
        static bool IsOfType(const Json& val);
        template<class T>
        [[nodiscard]] bool IsOf() const;

        template<class T>
        static T& AsType(Json& val);
        template<class T>
        T& As();
        template<class T>
        [[nodiscard]] const T& As() const;

        [[nodiscard]] bool operator==(const Json& other) const;

        //! @brief Tries to parse the Json from a string.
        //! @param input the text to parse.
        //! @param copyData if this is false then the json will use references to text.
        //! @return A Json if the parse success, std::nullopt if it fails.
        static std::optional<Json> Parse(std::string_view input, bool copyData = true);


        //! @brief Return the element with this index/key if this is an Object or Array. Return std::nullopt otherwise.
        OptRefValWrapper Get(Key key);
        //! @brief Return the element with this index/key if this is an Object or Array. Return std::nullopt otherwise.
        [[nodiscard]] OptCRefValWrapper Get(Key key) const;
        //! @brief Return a copy of the element with this index/key if this is an Object or Array. Return std::nullopt
        //! otherwise.
        [[nodiscard]] OptValWrapper GetCopy(Key key) const;
        //! @brief Move the element with this index/key if this is an Object or Array. Return std::nullopt otherwise.
        OptValWrapper GetAndMove(Key key) &&;


        //! @brief Same as successive calls to Get. Return std::nullopt at the first fail.
        OptRefValWrapper Find(std::span<const Key> keys);
        //! @brief Same as successive calls to Get. Return std::nullopt at the first fail.
        [[nodiscard]] OptCRefValWrapper Find(std::span<const Key> keys) const;

        //! @brief Same as successive calls to Get. Return std::nullopt at the first fail.
        RefValWrapperOrNull FindOrNull(std::span<const Key> keys);
        //! @brief Same as successive calls to Get. Return std::nullopt at the first fail.
        [[nodiscard]] CRefValWrapperOrNull FindOrNull(std::span<const Key> keys) const;

        //! @brief Same as successive calls to GetCopy. Return std::nullopt at the first fail.
        [[nodiscard]] OptValWrapper FindCopy(std::span<const Key> keys) const;
        //! @brief Same as successive calls to GetCopy. Return std::nullopt at the first fail.
        [[nodiscard]] ValWrapperOrNull FindOrNullCopy(std::span<const Key> keys) const;


        //! @brief Same as successive calls to GetCopy. Return std::nullopt at the first fail.
        OptValWrapper FindAndMove(std::span<const Key> key) &&;
        // //! @brief Same as successive calls to GetCopy. Return std::nullopt at the first fail.
        // ValWrapperOrNull FindOrNullAndMove(std::span<const Key> key) &&;
    private:
        Value _value;
    };

    inline static constexpr Null NullV{};
    inline static Json NullJ{};
}


#include <Thoth/NJson/Json.tpp>

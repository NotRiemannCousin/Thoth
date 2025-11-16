// ReSharper disable CppPassValueParameterByConstReference

#include <functional>
#include <algorithm>
#include <bitset>
#include <expected>
#include <ranges>

#include <Thoth/Json/Json.hpp>
#include <Thoth/String/UnicodeView.hpp>

using namespace Thoth::Json;


#ifdef DENSE_DEBUG_JSON
#define DEBUG_PRINT(MSG) std::println(MSG);
#else
#define DEBUG_PRINT(MSG)
#endif


#pragma region JsonVal
static Json::JsonVal::Value I_CloneValue(const Json::JsonVal::Value& v) {
    return std::visit([]<typename Type>(Type const& x) -> Json::JsonVal::Value {
        using T = std::remove_cvref_t<Type>;
        if constexpr (std::same_as<T, std::unique_ptr<Json>>)
            return Json::JsonVal::Value{ std::make_unique<Json>(*x) };
        else
            return Json::JsonVal::Value{ x };
    }, v);
}


Json::JsonVal::JsonVal(Json&& child)      : _value{ std::make_unique<Json>(std::move(child)) } {
    DEBUG_PRINT("JsonVal => Json&& child");
 }

Json::JsonVal::JsonVal(const Json& child) : _value{ std::make_unique<Json>(child) } {
    DEBUG_PRINT("JsonVal => const Json& child");
 }


Json::JsonVal::JsonVal(Value&& newValue) noexcept : _value{ std::move(newValue) } {
    DEBUG_PRINT("JsonVal => Value&& newValue");
 }

Json::JsonVal::JsonVal(const Value& newValue)     : _value{ I_CloneValue(newValue) } {
    DEBUG_PRINT("JsonVal => const Value& newValue");
 }

Json::JsonVal::JsonVal(JsonVal&& other) noexcept  : _value{ std::move(other._value) } {
    DEBUG_PRINT("JsonVal => JsonVal&& other");
 }

Json::JsonVal::JsonVal(const JsonVal& other)      : _value{ I_CloneValue(other._value) } {
    DEBUG_PRINT("JsonVal => const JsonVal& other");
 }


Json::JsonVal& Json::JsonVal::operator=(Json&& other) {
    _value = std::make_unique<Json>(std::move(other));
    DEBUG_PRINT("JsonVal operator => Json&& child");
    return *this;
}

Json::JsonVal& Json::JsonVal::operator=(const Json& other) {
    _value = std::make_unique<Json>(other);
    DEBUG_PRINT("JsonVal operator => const Json& child");

    return *this;
}


Json::JsonVal& Json::JsonVal::operator=(Value&& newValue) noexcept {
    _value = std::move(newValue);
    DEBUG_PRINT("JsonVal operator => Value&& newValue");

    return *this;
}

Json::JsonVal& Json::JsonVal::operator=(const Value& newValue)     {
    _value = I_CloneValue(newValue);
    DEBUG_PRINT("JsonVal operator => const Value& newValue");

    return *this;
}

Json::JsonVal& Json::JsonVal::operator=(JsonVal&& other) noexcept  {
    _value = std::move(other._value);
    DEBUG_PRINT("JsonVal operator => JsonVal&& other");

    return *this;
}

Json::JsonVal& Json::JsonVal::operator=(const JsonVal& other)      {
    _value = I_CloneValue(other._value);
    DEBUG_PRINT("JsonVal operator => const JsonVal& other");

    return *this;
}


Json::JsonVal::operator Value&() { return _value; }

Json::JsonVal::operator const Value&() const { return _value; }


bool Json::JsonVal::operator==(const JsonVal& other) const {
    return std::visit([&]<class T>(const T& val){
            if constexpr (std::same_as<T, Object>)
                return std::holds_alternative<T>(other._value)&&  *std::get<T>(other._value) == *val;
            else
                return std::holds_alternative<T>(other._value)&&  std::get<T>(other._value) == val;
        }, _value);
}

Json::~Json() {
    DEBUG_PRINT("~Json");
}

#pragma endregion


#pragma region Json


Json::Json(const Json& other) {
    DEBUG_PRINT("Json => const JsonVal& other");
    _pairs = other._pairs;
    _buffer = other._buffer;
    _bufferView = other._bufferView;
    // If the buffer isn't user managed carries the reference, so when you delete the parent but not the children the
    // Cow still works. Yeah, given the necessity of `_bufferView` (user managed buffers) the `_buffer` itself it's
    // kinda dummy, it just holds the reference.
}

Json::Json(Json&& other) noexcept : _buffer{ std::move(other._buffer) }, _bufferView{ other._bufferView },
    _pairs{ std::move(other._pairs) } {
    DEBUG_PRINT("Json => JsonVal&& other");
}

Json& Json::operator=(const Json& other) {
    DEBUG_PRINT("Json equals operator => const JsonVal& other");
    _pairs = other._pairs;
    _buffer = other._buffer;
    _bufferView = other._bufferView;

    return *this;
}

Json& Json::operator=(Json&& other) noexcept {
    _pairs = std::move(other._pairs);
    _buffer = std::move(other._buffer);
    _bufferView = other._bufferView;
    DEBUG_PRINT("Json equals operator => const JsonVal& other");
    return *this;
}

Json::Json(MapType&& initAs) : _pairs{ std::move(initAs) } { }

Json::Json(const std::initializer_list<JsonPair>& init) : _pairs(init) { }

#define ADVANCE_IF(predicate) do {                                         \
        const char* ptr = input.data();                                    \
        const char* end = ptr + input.size();                              \
        while (ptr != end && (predicate)) ++ptr; /* SIMD someday */        \
            if (ptr == end) [[unlikely]] return false;                     \
            input.remove_prefix(static_cast<size_t>(ptr - input.data()));  \
    } while (0)

#define ADVANCE_IF_FUNC(func, predicate) do {                              \
        const char* ptr = input.data();                                    \
        const char* end = ptr + input.size();                              \
        while (ptr != end && func(predicate)) ++ptr; /* SIMD someday */        \
            if (ptr == end) [[unlikely]] return false;                     \
            input.remove_prefix(static_cast<size_t>(ptr - input.data()));  \
    } while (0)

#define ADVANCE_SPACES() ADVANCE_IF(*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')

std::optional<Json> Json::Parse(std::string_view text, bool copyData) {
    Json json;

    if (!copyData)
        json._bufferView = text;
    else {
        json._buffer = std::make_shared<std::string>(text);
        json._bufferView = *json._buffer;
    }

    std::string_view input{ json._bufferView };

    if (!ParseUnchecked(input, json, json))
        return std::nullopt;

#define return break; // I hate to do it but performance matters
    ADVANCE_SPACES();
#undef return

    if (input.empty())
        return json;

    return std::nullopt;
}

#pragma region I hate to love c preprocessor

#define CASE_OPEN_STRING   case '"':
#define CASE_OPEN_NUMBER   case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':case '-':
#define CASE_OPEN_OBJECT   case '{':
#define CASE_OPEN_BOOLEAN  case 'f':case 't':
#define CASE_OPEN_NULLABLE case 'n':
#define CASE_OPEN_ARRAY    case '[':

#pragma endregion


#pragma region Read functions

static bool I_ReadString(std::string_view& input, auto& val) {
    if (*input.data() != '"')
        return false;
    input.remove_prefix(1);

    const auto start{ input.cbegin() };


    size_t iterations{ 1 };
    for (;; iterations++) {
        ADVANCE_IF(*ptr != '\"' && *ptr != '\\');

        if (input.empty()) [[unlikely]]
            return false;

        if (*input.data() == '"')
            break;

        if (input.size() <= 2) // ignoring \*
            return false;
        input.remove_prefix(2);
        // It doesn't matter which char is, discard the '\\' and what goes after it. The check will performed after.
    }
    std::string_view strRef{ &*start, &*input.begin() };
    input.remove_prefix(1);

    // Well, it doesn't make sense to check for all chars because just strings can have UTF-8 chars.
    // Moreover, now the cache will not blame on me again.
    if (std::ranges::any_of(strRef, [](const auto c){ return c & 0x80; }) &&
        !Thoth::String::Utf8View::IsValid(std::bit_cast<std::u8string_view>(strRef)))
        return false;

    if (iterations == 1) {
        val = String::FromRef(strRef);
        return true;
    }

    std::string str;
    str.reserve(strRef.size());

    while (true) {
        const size_t pos{ strRef.find('\\') };

        if (pos == std::string::npos)
            break;

        str.append_range(std::string_view{ strRef.data(), pos });
        strRef.remove_prefix(pos + 1);


        switch (*strRef.data()) {
            case 'u' :
                str += "\\u"; break;
                // if (strRef.size() < 3)
                //     return false;

                // break;
            case '\\': str.push_back('\\'); break;
            case '"' : str.push_back('\"'); break;
            case 'n' : str.push_back('\n'); break;
            case 'r' : str.push_back('\r'); break;
            case 't' : str.push_back('\t'); break;

            default: return false;
        }
    }
    str.append_range(strRef);

    val = String::FromOwned(std::move(str));
    return true;
}
static bool I_ReadNumber(std::string_view& input, auto& val) {
    const auto openValNumber{ input.data() };
    constexpr auto validChars = []{
        std::bitset<256> res{};

        for (const char c :std::string_view{ "01234567899.-eE" })
            res.set(c);

        return res;
    }();

    ADVANCE_IF(validChars[*ptr]);


    const auto closeValNumber{ input.data() };

    long double number{};

    auto [ptr, err] {
        std::from_chars(
        & *openValNumber,
        & *closeValNumber,
                number
            )};
    if (err != std::errc{} || ptr != closeValNumber)
        return false;

    val = number;
    return true;
};
static bool I_ReadObject(std::string_view& input, auto& val, const Json& parent) {
    Json newJson;
    if (!Json::ParseUnchecked(input, newJson, parent))
        return false;

    if (input.empty())
        return false;
    val = std::move(newJson);
    return true;
}
static bool I_ReadBool(std::string_view& input, auto& val) {
    if (std::ranges::starts_with(input, std::string_view{ "true" }))
        input.remove_prefix(4), val = true;
    else if (std::ranges::starts_with(input, std::string_view{ "false" }))
        input.remove_prefix(5), val = false;
    else
        return false;
    return true;
};
static bool I_ReadNull(std::string_view& input, auto& val) {
    if (std::ranges::starts_with(input, std::string_view{ "null" }))
        input.remove_prefix(4);
    else
        return false;

    val = NullV;
    return true;
};
static bool I_ReadArray(std::string_view& input, auto& val, const Json& parent) {
    if (*input.data() != '[')
        return false;

    Array array{};

    while (*input.data() != ']') {
        input.remove_prefix(1);
        ADVANCE_SPACES();

        array.emplace_back(NullV);

        bool success{};
        switch (*input.data()) {
            CASE_OPEN_STRING   success = I_ReadString(input, array.back());         break;
            CASE_OPEN_NUMBER   success = I_ReadNumber(input, array.back());         break;
            CASE_OPEN_OBJECT   success = I_ReadObject(input, array.back(), parent); break;
            CASE_OPEN_BOOLEAN  success = I_ReadBool(  input, array.back());         break;
            CASE_OPEN_NULLABLE success = I_ReadNull(  input, array.back());         break;
            CASE_OPEN_ARRAY    success = I_ReadArray( input, array.back(), parent); break;
            default: return false;
        }
        if (!success)
            return false;

        ADVANCE_SPACES();

        if(*input.data() != ',' &&  *input.data() != ']')
            return false;
    }
    input.remove_prefix(1);

    val = std::move(array);
    return true;
}

#pragma endregion


bool Json::ParseUnchecked(std::string_view& input, Json& json, const Json& parent) {

    ADVANCE_SPACES();

    if (*input.data() != '{')
        return false;

    while (*input.data() != '}') {
        input.remove_prefix(1);

        ADVANCE_SPACES();

        String key;
        if (!I_ReadString(input, key))
            return false;

        ADVANCE_SPACES();

        if (*input.data() != ':')
            return false;
        input.remove_prefix(1);

        ADVANCE_SPACES();

        auto [newItem, _]{ json._pairs.try_emplace(key.AsOwned(), NullV ) };

        bool success{};
        switch (*input.data()) {
            CASE_OPEN_STRING   success = I_ReadString(input, newItem->second);         break;
            CASE_OPEN_NUMBER   success = I_ReadNumber(input, newItem->second);         break;
            CASE_OPEN_OBJECT   success = I_ReadObject(input, newItem->second, parent); break;
            CASE_OPEN_NULLABLE success = I_ReadNull(  input, newItem->second);         break;
            CASE_OPEN_BOOLEAN  success = I_ReadBool(  input, newItem->second);         break;
            CASE_OPEN_ARRAY    success = I_ReadArray( input, newItem->second, parent); break;
            default: return false;
        }
        if (!success)
            return false;

        ADVANCE_SPACES();

        if(*input.data() != ',' && *input.data() != '}')
            return false;
    }
    input.remove_prefix(1);
    json._bufferView = parent._bufferView;
    json._buffer     = parent._buffer;


    return true;
}


#undef ADVANCE_IF
#undef ADVANCE_IF_FUNC
#undef ADVANCE_SPACES

#undef CASE_OPEN_STRING
#undef CASE_OPEN_NUMBER
#undef CASE_OPEN_OBJECT
#undef CASE_OPEN_BOOLEAN
#undef CASE_OPEN_NULLABLE
#undef CASE_OPEN_ARRAY

bool Json::Exists(JsonKeyRef key) const {
    return _pairs.contains(key);
}

bool Json::Exists(JsonPairRef p) const {
    return Exists(p.first, p.second);
}

bool Json::Exists(JsonKeyRef key, JsonValRef val) const {
    const auto it{ _pairs.find(key) };

    return it != _pairs.end()&& it->second == val;
}

void Json::Set(JsonPairRef p) {
    Set(p.first, p.second);
}

void Json::Set(JsonKeyRef key, JsonValRef val) {
    _pairs.try_emplace(key, val);
}

bool Json::Remove(JsonKeyRef key) {
    return _pairs.erase(key);
}


bool Json::SetIfNull(JsonPairRef p) {
    return SetIfNull(p.first, p.second);
}

bool Json::SetIfNull(JsonKeyRef key, JsonValRef val) {
    auto [_, tookPlace]{ _pairs.try_emplace(key, val) };

    return tookPlace;
}


Json::OptRefValWrapper Json::Get(JsonKeyRef key) {
    const auto it{ _pairs.find(key) };

    if (it == _pairs.end())
        return std::nullopt;
    return it->second;
}

Json::OptCRefValWrapper Json::Get(JsonKeyRef key) const {
    const auto it{ _pairs.find(key) };

    if (it == _pairs.end())
        return std::nullopt;
    return it->second;
}

Json::CRefValWrapperOrNull Json::GetOrNull(JsonKeyRef key) const {
    return Get(key).value_or(NullJ);
}


Json::OptValWrapper Json::GetCopy(JsonKeyRef key) const {
    const auto it{ _pairs.find(key) };

    if (it == _pairs.end())
        return std::nullopt;
    return it->second;
}

Json::ValWrapperOrNull Json::GetOrNullCopy(JsonKeyRef key) const {
    return GetCopy(key).value_or(NullJ);
}


Json::OptValWrapper Json::GetMove(JsonKeyRef key) && {
    if (auto it = _pairs.find(key); it != _pairs.end())
        return std::move(it->second);
    return std::nullopt;
}

Json::ValWrapperOrNull Json::GetOrNullMove(JsonKeyRef key) && {
    if (auto it = _pairs.find(key); it != _pairs.end())
        return std::move(it->second);
    return std::unexpected{ NullV };
}


void Json::Clear() {
    _pairs.clear();
}

size_t Json::Size() const {
    return _pairs.size();
}

bool Json::Empty() const {
    return _pairs.empty();
}

Json::JsonVal& Json::operator[](JsonKeyRef key) {
    const auto [it, _]{ _pairs.try_emplace(key, NullV) };

    return it->second;
}

const Json::JsonVal& Json::operator[](JsonKeyRef key) const {
    const auto it{ _pairs.find(key) };
    return it != _pairs.end() ? it->second : NullJ;
}

bool Json::operator==(const Json& other) const {
    return _pairs == other._pairs;
}

#pragma endregion

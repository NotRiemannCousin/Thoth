#include <functional>
#include <algorithm>
#include <bitset>
#include <expected>
#include <ranges>

#include <Thoth/Json/Json.hpp>
#include <Thoth/String/UnicodeView.hpp>

using namespace Thoth::Json;


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


Json::JsonVal::JsonVal(Json&& child)      : value{ std::make_unique<Json>(std::move(child)) } { }

Json::JsonVal::JsonVal(const Json& child) : value{ std::make_unique<Json>(child) } { }


Json::JsonVal::JsonVal(Value&& newValue) noexcept : value{std::move(newValue) } { }

Json::JsonVal::JsonVal(const Value& newValue)     : value{I_CloneValue(newValue) } { }

Json::JsonVal::JsonVal(JsonVal&& other) noexcept  : value{std::move(other.value) } { }

Json::JsonVal::JsonVal(const JsonVal& other)      : value{I_CloneValue(other.value) } { }


Json::JsonVal& Json::JsonVal::operator=(Json&& child) { value = std::make_unique<Json>(std::move(child)); return *this; } // Yes, 121 line width

Json::JsonVal& Json::JsonVal::operator=(const Json& child) { value = std::make_unique<Json>(child);       return *this; }


Json::JsonVal& Json::JsonVal::operator=(Value&& newValue) noexcept { value = std::move(newValue);       return *this; }

Json::JsonVal& Json::JsonVal::operator=(const Value& newValue)     { value = I_CloneValue(newValue);    return *this; }

Json::JsonVal& Json::JsonVal::operator=(JsonVal&& other) noexcept  { value = std::move(other.value);    return *this; }

Json::JsonVal& Json::JsonVal::operator=(const JsonVal& other)      { value = I_CloneValue(other.value); return *this; }


Json::JsonVal::operator Value&() { return value; }

Json::JsonVal::operator const Value&() const { return value; }


bool Json::JsonVal::operator==(const JsonVal& other) const {
    return std::visit([&]<class T>(const T& val){
            if constexpr (std::same_as<T, Object>)
                return std::holds_alternative<T>(other.value)&&  *std::get<T>(other.value) == *val;
            else
                return std::holds_alternative<T>(other.value)&&  std::get<T>(other.value) == val;
        }, value);
}

#pragma endregion


#pragma region Json


Json::Json(const Json &other) {
    if (other.bufferView.data() == other.buffer.data())
        bufferView = buffer = other.buffer;
    else
        bufferView = other.bufferView;

    pairs = other.pairs;

    namespace vs = std::views;
    for (auto& val : pairs
            | vs::values
            | vs::filter(&JsonVal::IsOfType<String>)
            | vs::transform(&JsonVal::AsType<String>)
            | vs::filter(&String::IsRefType))
        val.SetRef({ bufferView.data(), val.AsRef().size() });
}

Json& Json::operator=(const Json &other) {
    if (other.bufferView.data() == other.buffer.data())
        bufferView = buffer = other.buffer;
    else
        bufferView = other.bufferView;

    pairs = other.pairs;

    namespace vs = std::views;
    for (auto& val : pairs
            | vs::values
            | vs::filter(&JsonVal::IsOfType<String>)
            | vs::transform(&JsonVal::AsType<String>)
            | vs::filter(&String::IsRefType))
        val.SetRef({ bufferView.data(), val.AsRef().size() });

    return *this;
}

Json::Json(MapType&& initAs) : pairs{ std::move(initAs) } { }

Json::Json(const std::initializer_list<JsonPair>& init) : pairs(init) { }

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

    if (copyData)
        json.bufferView = json.buffer = text;
    else
        json.bufferView = text;

    std::string_view input{ json.bufferView };

    if (!ParseUnchecked(input, json))
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
                // if (strRef.size() < 3)
                //     return false;

                break;
            case '\\':
            case '"': str.push_back(*strRef.data());  break;
            case 'n': str.push_back('\n');            break;
            case 'r': str.push_back('\r');            break;
            case 't': str.push_back('\t');            break;

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
static bool I_ReadObject(std::string_view& input, auto& val) {
    Json newJson;
    if (!Json::ParseUnchecked(input, newJson))
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
static bool I_ReadArray(std::string_view& input, auto& val) {
    if (*input.data() != '[')
        return false;

    Array array{};

    while (*input.data() != ']') {
        input.remove_prefix(1);
        ADVANCE_SPACES();

        array.emplace_back(NullV);

        bool success{};
        switch (*input.data()) {
            CASE_OPEN_STRING   success = I_ReadString(input, array.back()); break;
            CASE_OPEN_NUMBER   success = I_ReadNumber(input, array.back()); break;
            CASE_OPEN_OBJECT   success = I_ReadObject(input, array.back()); break;
            CASE_OPEN_BOOLEAN  success = I_ReadBool(  input, array.back()); break;
            CASE_OPEN_NULLABLE success = I_ReadNull(  input, array.back()); break;
            CASE_OPEN_ARRAY    success = I_ReadArray( input, array.back()); break;
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


bool Json::ParseUnchecked(std::string_view& input, Json& json) {

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

        auto [newItem, _]{ json.pairs.try_emplace(key.AsOwned(), NullV ) };

        bool success{};
        switch (*input.data()) {
            CASE_OPEN_STRING   success = I_ReadString(input, newItem->second); break;
            CASE_OPEN_NUMBER   success = I_ReadNumber(input, newItem->second); break;
            CASE_OPEN_OBJECT   success = I_ReadObject(input, newItem->second); break;
            CASE_OPEN_NULLABLE success = I_ReadNull(  input, newItem->second); break;
            CASE_OPEN_BOOLEAN  success = I_ReadBool(  input, newItem->second); break;
            CASE_OPEN_ARRAY    success = I_ReadArray( input, newItem->second); break;
            default: return false;
        }
        if (!success)
            return false;

        ADVANCE_SPACES();

        if(*input.data() != ',' && *input.data() != '}')
            return false;
    }
    input.remove_prefix(1);


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
    return pairs.contains(key);
}

bool Json::Exists(JsonPairRef p) const {
    return Exists(p.first, p.second);
}

bool Json::Exists(JsonKeyRef key, JsonValRef val) const {
    const auto it{ pairs.find(key) };

    return it != pairs.end()&& it->second == val;
}

void Json::Set(JsonPairRef p) {
    Set(p.first, p.second);
}

void Json::Set(JsonKeyRef key, JsonValRef val) {
    pairs.try_emplace(key, val);
}

bool Json::Remove(JsonKeyRef key) {
    return pairs.erase(key);
}


bool Json::SetIfNull(JsonPairRef p) {
    return SetIfNull(p.first, p.second);
}

bool Json::SetIfNull(JsonKeyRef key, JsonValRef val) {
    auto [_, tookPlace]{ pairs.try_emplace(key, val) };

    return tookPlace;
}

Json::OptRefValWrapper Json::Get(JsonKeyRef key) {
    const auto it{ pairs.find(key) };

    if (it == pairs.end())
        return std::nullopt;
    return it->second;
}

Json::OptCRefValWrapper Json::Get(JsonKeyRef key) const {
    const auto it{ pairs.find(key) };

    if (it == pairs.end())
        return std::nullopt;
    return it->second;
}

Json::CRefValWrapperOrNull Json::GetOrNull(JsonKeyRef key) const {
    return Get(key).value_or(NullJ);
}

void Json::Clear() {
    pairs.clear();
}

size_t Json::Size() const {
    return pairs.size();
}

bool Json::Empty() const {
    return pairs.empty();
}

Json::JsonVal& Json::operator[](JsonKeyRef key) {
    const auto [it, _]{ pairs.try_emplace(key, NullV) };

    return it->second;
}

const Json::JsonVal& Json::operator[](JsonKeyRef key) const {
    const auto it{ pairs.find(key) };
    return it != pairs.end() ? it->second : NullJ;
}

bool Json::operator==(const Json& other) const {
    return pairs == other.pairs;
}

#pragma endregion

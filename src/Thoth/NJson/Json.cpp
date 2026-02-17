// ReSharper disable CppPassValueParameterByConstReference

#include <algorithm>
#include <bitset>
#include <execution>
#include <expected>

#include <Thoth/String/UnicodeViewer.hpp>
#include <Thoth/Http/RequestError.hpp>
#include <Thoth/Utils/Functional.hpp>
#include <Thoth/NJson/Json.hpp>

using namespace Thoth::NJson;
using namespace Thoth::Http;


#ifdef DENSE_DEBUG_JSON
#include <print>
#define DEBUG_PRINT(MSG) std::println(MSG);
#else
#define DEBUG_PRINT(MSG)
#endif


static Json::Value I_CloneValue(const Json::Value& v) {
    return std::visit([]<typename Type>(Type const& x) -> Json::Value {
        using T = std::remove_cvref_t<Type>;
        if constexpr (std::same_as<T, Object>)
            return Json::Value{ std::make_unique<JsonObject>(*x) };
        // else if constexpr (std::same_as<T, Array>)
        //     return Json::Value{ std::make_unique<std::vector<Json>>(*x) };
        else
            return Json::Value{ x };
    }, v);
}


Json::Json(JsonObject&& child)      : _value{ std::make_unique<JsonObject>(std::move(child)) } {
    DEBUG_PRINT("JsonVal => Json&& child");
 }

Json::Json(const Array& child) : _value{ child } {
    DEBUG_PRINT("JsonVal => const Array& child");
}

Json::Json(Array&& child) : _value{ std::move(child) } {
    DEBUG_PRINT("JsonVal => Array&& child");
}

Json::Json() {
    DEBUG_PRINT("JsonVal => Default");
}

Json::Json(const JsonObject& child) : _value{ std::make_unique<JsonObject>(child) } {
    DEBUG_PRINT("JsonVal => const Json& child");
 }


Json::Json(Value&& newValue) noexcept : _value{ std::move(newValue) } {
    DEBUG_PRINT("JsonVal => Value&& newValue");
 }

Json::Json(const Value& newValue)     : _value{ I_CloneValue(newValue) } {
    DEBUG_PRINT("JsonVal => const Value& newValue");
 }

Json::Json(Json&& other) noexcept  : _value{ std::move(other._value) } {
    DEBUG_PRINT("JsonVal => JsonVal&& other");
 }

Json::Json(const Json& other)      : _value{ I_CloneValue(other._value) } {
    DEBUG_PRINT("JsonVal => const JsonVal& other");
}

Json::Json(bool b) : _value{ b } {
    DEBUG_PRINT("JsonVal => bool b");
}




Json& Json::operator=(JsonObject&& other) {
    _value = std::make_unique<JsonObject>(std::move(other));
    DEBUG_PRINT("JsonVal operator => Json&& child");
    return *this;
}

Json& Json::operator=(const JsonObject& other) {
    _value = std::make_unique<JsonObject>(other);
    DEBUG_PRINT("JsonVal operator => const Json& child");

    return *this;
}

Json & Json::operator=(const Array &child) {
    _value = child;
    DEBUG_PRINT("JsonVal operator => const array& child");

    return *this;
}

Json & Json::operator=(Array &&child) {
    _value = std::move(child);
    DEBUG_PRINT("JsonVal operator => const array& child");

    return *this;
}

Json& Json::operator=(Value&& newValue) noexcept {
    _value = std::move(newValue);
    DEBUG_PRINT("JsonVal operator => Value&& newValue");

    return *this;
}

Json& Json::operator=(const Value& newValue) {
    _value = I_CloneValue(newValue);
    DEBUG_PRINT("JsonVal operator => const Value& newValue");

    return *this;
}

Json& Json::operator=(Json&& other) noexcept {
    _value = std::move(other._value);
    DEBUG_PRINT("JsonVal operator => JsonVal&& other");

    return *this;
}

Json& Json::operator=(const Json& other) {
    if (this == &other)
        return *this;

    _value = I_CloneValue(other._value);
    DEBUG_PRINT("JsonVal operator => const JsonVal& other");

    return *this;
}

Json& Json::operator=(bool other) {
    _value = other;
    return *this;
}


Json::operator Value&() { return _value; }

Json::operator const Value&() const { return _value; }


bool Json::operator==(const Json& other) const {
    return std::visit([&]<class T>(const T& val){
            if constexpr (std::same_as<T, Object>)
                return std::holds_alternative<T>(other._value) && *std::get<T>(other._value) == *val;
            else
                return std::holds_alternative<T>(other._value) && std::get<T>(other._value) == val;
        }, _value);
}


#pragma region I hate to love c preprocessor
// This section is dedicated to this cursed amazing feature that deserves
// to be deprecated/context constrained 10 years ago

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
        while (ptr != end && func(predicate)) ++ptr; /* SIMD someday */    \
            if (ptr == end) [[unlikely]] return false;                     \
            input.remove_prefix(static_cast<size_t>(ptr - input.data()));  \
    } while (0)

#define ADVANCE_SPACES() ADVANCE_IF(*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')


#define CASE_OPEN_STRING   case '"':
#define CASE_OPEN_NUMBER   case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':case '-':
#define CASE_OPEN_OBJECT   case '{':
#define CASE_OPEN_BOOLEAN  case 'f':case 't':
#define CASE_OPEN_NULLABLE case 'n':
#define CASE_OPEN_ARRAY    case '[':

#pragma endregion

#pragma region Read functions

static bool S_DecodeUtf16(std::string_view& s, std::string& out) {
    constexpr auto hex = [](const char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    };

    auto readU = [&](uint16_t& v) -> bool {
        if (s.size() < 5 || s[0] != 'u') return false;
        uint16_t x{};
        for (int i = 1; i < 5; i++) {
            const int h{ hex(s[i]) };
            if (h < 0) return false;
            x = (x<<4) | h;
        }
        v = x;
        s.remove_prefix(5);
        return true;
    };

    uint16_t h;
    if (!readU(h)) return false;

    uint32_t code{ h };

    // surrogate?
    if (0xD800 <= h && h <= 0xDBFF) {
        if (s.size() < 1 || s[0] != '\\') return false;
        s.remove_prefix(1);

        uint16_t l;
        if (!readU(l)) return false;
        if (l < 0xDC00 || l > 0xDFFF) return false;
        code = 0x10000 + (((h - 0xD800) << 10) | (l - 0xDC00));
    }

#define char static_cast<char>
    if (code < 0x80) {
        out.push_back(char(code));
    } else if (code < 0x800) {
        out.push_back(char(0xC0 | (code>>6)));
        out.push_back(char(0x80 | (code&63)));
    } else if (code < 0x10000) {
        out.push_back(char(0xE0 | (code>>12)));
        out.push_back(char(0x80 | ((code>>6)&63)));
        out.push_back(char(0x80 | (code&63)));
    } else {
        out.push_back(char(0xF0 | (code>>18)));
        out.push_back(char(0x80 | ((code>>12)&63)));
        out.push_back(char(0x80 | ((code>>6)&63)));
        out.push_back(char(0x80 | (code&63)));
    }
#undef char

    return true;
}


static bool Details::ReadString(std::string_view& input, auto& val, const BufferInfo& info) {
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
        val = String::FromRef({strRef, info.buffer});
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
            case 'u' : if (!S_DecodeUtf16(strRef, str)) return false; break;
            case '\\': str.push_back('\\'); strRef.remove_prefix(1);  break;
            case '"' : str.push_back('\"'); strRef.remove_prefix(1);  break;
            case 'n' : str.push_back('\n'); strRef.remove_prefix(1);  break;
            case 'r' : str.push_back('\r'); strRef.remove_prefix(1);  break;
            case 't' : str.push_back('\t'); strRef.remove_prefix(1);  break;

            default: return false;
        }
    }
    str.append_range(strRef);

    val = String::FromOwned(std::move(str));
    return true;
}
static bool Details::ReadNumber(std::string_view& input, auto& val) {
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
}
static bool Details::ReadObject(std::string_view& input, auto& val, const BufferInfo& info) {
    JsonObject json;

    ADVANCE_SPACES();

    if (*input.data() != '{')
        return false;

    while (*input.data() != '}') {
        input.remove_prefix(1);

        ADVANCE_SPACES();

        if (json.Size() == 0 && *input.data() == '}')
            break;

        String key;
        if (!ReadString(input, key, info))
            return false;

        ADVANCE_SPACES();

        if (*input.data() != ':')
            return false;
        input.remove_prefix(1);

        ADVANCE_SPACES();

        auto [newItem, _]{ json._pairs.try_emplace(key.AsOwned(), NullV ) };

        bool success{};
        switch (*input.data()) {
            CASE_OPEN_STRING   success = ReadString(input, newItem->second, info); break;
            CASE_OPEN_NUMBER   success = ReadNumber(input, newItem->second);       break;
            CASE_OPEN_OBJECT   success = ReadObject(input, newItem->second, info); break;
            CASE_OPEN_BOOLEAN  success = ReadBool(  input, newItem->second);       break;
            CASE_OPEN_NULLABLE success = ReadNull(  input, newItem->second);       break;
            CASE_OPEN_ARRAY    success = ReadArray( input, newItem->second, info); break;
            default: return false;
        }
        if (!success)
            return false;

        ADVANCE_SPACES();

        if(*input.data() != ',' && *input.data() != '}')
            return false;
    }
    input.remove_prefix(1);

    val = std::move(json);
    return true;
}
static bool Details::ReadBool(std::string_view& input, auto& val) {
    if (std::ranges::starts_with(input, std::string_view{ "true" }))
        input.remove_prefix(4), val = true;
    else if (std::ranges::starts_with(input, std::string_view{ "false" }))
        input.remove_prefix(5), val = false;
    else
        return false;
    return true;
}
static bool Details::ReadNull(std::string_view& input, auto& val) {
    if (std::ranges::starts_with(input, std::string_view{ "null" }))
        input.remove_prefix(4);
    else
        return false;

    val = NullV;
    return true;
};
static bool Details::ReadArray(std::string_view& input, auto& val, const BufferInfo& info) {
    if (*input.data() != '[')
        return false;

    Array array{};

    while (*input.data() != ']') {
        input.remove_prefix(1);
        ADVANCE_SPACES();

        if (array.size() == 0 && *input.data() == ']')
            break;

        array.emplace_back(NullV);

        bool success{};
        switch (*input.data()) {
            CASE_OPEN_STRING   success = ReadString(input, array.back(), info); break;
            CASE_OPEN_NUMBER   success = ReadNumber(input, array.back());       break;
            CASE_OPEN_OBJECT   success = ReadObject(input, array.back(), info); break;
            CASE_OPEN_BOOLEAN  success = ReadBool(  input, array.back());       break;
            CASE_OPEN_NULLABLE success = ReadNull(  input, array.back());       break;
            CASE_OPEN_ARRAY    success = ReadArray( input, array.back(), info); break;
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


std::expected<Json, RequestError> Json::Parse(std::string_view input) {
    return ParseText(input);
}

std::expected<Json, RequestError> Json::ParseText(std::string_view input, bool copyData, bool checkFinal) {
    Details::BufferInfo info{};

    if (copyData) {
        info.buffer = std::make_shared<std::string>(input);
        info.bufferView = *info.buffer;
        input = info.bufferView;
    }
    else
        info.bufferView = input;

    const auto s_error = [&]() -> std::unexpected<RequestError>{
        return std::unexpected{ RequestError{ JsonParseError{ info.bufferView.size() - input.size(), input[0] } } };
    };

#define return return s_error(); // I hate to do it
    ADVANCE_SPACES();
#undef return

    Json json{};
    bool success{};

    switch (input[0]){
        CASE_OPEN_STRING   success = Details::ReadString(input, json, info); break;
        CASE_OPEN_NUMBER   success = Details::ReadNumber(input, json);       break;
        CASE_OPEN_OBJECT   success = Details::ReadObject(input, json, info); break;
        CASE_OPEN_NULLABLE success = Details::ReadNull(  input, json);       break;
        CASE_OPEN_BOOLEAN  success = Details::ReadBool(  input, json);       break;
        CASE_OPEN_ARRAY    success = Details::ReadArray( input, json, info); break;
        default: return s_error();
    }
    if (!success)
        return s_error();

#define return json; // Oh god, no again
    if (checkFinal) {
        ADVANCE_SPACES();
    }
#undef return

    if (input.empty() || !checkFinal)
        return json;

    return s_error();
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


constexpr auto ResolveKeys = Thoth::Utils::Overloaded{
    [](auto& curr, const int index) -> bool {
        if (!Json::IsOfType<Array>(**curr))
            return false;

        // auto& arr{ Json::AsType<Array>(*curr) };
        auto& arr{ curr.value()->template As<Array>() };

        const int finalIndex{
            index >= 0
                ? index
                : static_cast<int>(arr.size() + index)
        };

        if (abs(finalIndex) >= arr.size())
            return false;


        curr = &arr.operator[](finalIndex);

        return true;
    },
    [](auto& curr, const std::string_view key) -> bool {
        if (!Json::IsOfType<Object>(**curr))
            return false;

        auto& obj{ curr.value()->template As<Object>() };
        auto it{ obj->Get(key) };

        if (!it)
            return false;

        curr = it;
        return true;
    }
};

OptRefValWrapper Json::Get(Key key) {
    OptRefValWrapper curr{ this };

    if (!std::visit([&](const auto& k){ return ResolveKeys(curr, k); }, key))
        return std::nullopt;

    return curr;
}

OptCRefValWrapper Json::Get(Key key) const {
    OptCRefValWrapper curr{ this };

    if (!std::visit([&](const auto& k){ return ResolveKeys(curr, k); }, key))
        return std::nullopt;

    return curr;
}

OptValWrapper Json::GetCopy(Key key) const {
    OptCRefValWrapper curr{ this };

    if (!std::visit([&](const auto& k){ return ResolveKeys(curr, k); }, key))
        return std::nullopt;

    return curr;
}

OptValWrapper Json::GetAndMove(Key key) && {
    OptRefValWrapper curr{ this };

    if (!std::visit([&](const auto& k) { return ResolveKeys(curr, k); }, key))
        return std::nullopt;

    return std::move(**curr);
}

OptRefValWrapper Json::Find(Keys keys) {
    OptRefValWrapper curr{ this };

    for (const auto& key : keys)
        if (!std::visit([&](const auto& k){ return ResolveKeys(curr, k); }, key))
            return std::nullopt;

    return curr;
}

OptCRefValWrapper Json::Find(Keys keys) const {
    OptCRefValWrapper curr{ this };

    for (const auto& key : keys)
        if (!std::visit([&](const auto& k){ return ResolveKeys(curr, k); }, key))
            return std::nullopt;

    return curr;
}


RefValWrapperOrNull Json::FindOrNull(Keys keys) {
    return Find(keys).value_or(&NullJ);
}

CRefValWrapperOrNull Json::FindOrNull(Keys keys) const {
    return Find(keys).value_or(&NullJ);
}

OptValWrapper Json::FindCopy(Keys keys) const {
    return **Find(keys);
}

ValWrapperOrNull Json::FindOrNullCopy(Keys keys) const {
    return FindCopy(keys).value_or(NullJ);
}

OptValWrapper Json::FindAndMove(Keys key) && {
    auto ptr{ Find(key) };
    if (!ptr) return NullJ;

    return std::move(**ptr);
}

// ValWrapperOrNull Json::FindOrNullAndMove(Keys keys) && {
//     Null dummy;
//     return FindAndMove(keys).value_or(std::move(dummy));
// }

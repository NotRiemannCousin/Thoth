// ReSharper disable CppPassValueParameterByConstReference

#include <algorithm>
#include <bitset>
#include <execution>
#include <expected>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <bit>

#include <Thoth/String/UnicodeViewer.hpp>
#include <Thoth/ThothError.hpp>
#include <Thoth/Utils/Functional.hpp>
#include <Thoth/NJson/Json.hpp>

using namespace Thoth::NJson;
using namespace Thoth::Http;
using Thoth::ThothError;

#ifdef DENSE_DEBUG_JSON
#include <print>
#define DEBUG_PRINT(MSG) std::println(MSG);
#else
#define DEBUG_PRINT(MSG)
#endif


static Json::Value CloneValue(const Json::Value& v) {
    return std::visit([]<class Type>(Type const& x) -> Json::Value {
        using T = std::remove_cvref_t<Type>;
        if constexpr (std::same_as<T, Object>)
            return Json::Value{ std::make_unique<JsonObject>(*x) };
        // else if constexpr (std::same_as<T, Array>)
        //     return Json::Value{ std::make_unique<std::vector<Json>>(*x) };
        else
            return Json::Value{ x };
    }, v);
}


Json::Json(JsonObject&& child)      : m_value{ std::make_unique<JsonObject>(std::move(child)) } {
    DEBUG_PRINT("JsonVal => Json&& child");
 }

Json::Json(const Array& child) : m_value{ child } {
    DEBUG_PRINT("JsonVal => const Array& child");
}

Json::Json(Array&& child) : m_value{ std::move(child) } {
    DEBUG_PRINT("JsonVal => Array&& child");
}

Json::Json() {
    DEBUG_PRINT("JsonVal => Default");
}

Json::Json(const JsonObject& child) : m_value{ std::make_unique<JsonObject>(child) } {
    DEBUG_PRINT("JsonVal => const Json& child");
 }


Json::Json(Value&& newValue) noexcept : m_value{ std::move(newValue) } {
    DEBUG_PRINT("JsonVal => Value&& newValue");
 }

Json::Json(const Value& newValue)     : m_value{ CloneValue(newValue) } {
    DEBUG_PRINT("JsonVal => const Value& newValue");
 }

Json::Json(Json&& other) noexcept  : m_value{ std::move(other.m_value) } {
    DEBUG_PRINT("JsonVal => JsonVal&& other");
 }

Json::Json(const Json& other)      : m_value{ CloneValue(other.m_value) } {
    DEBUG_PRINT("JsonVal => const JsonVal& other");
}

Json::Json(bool b) : m_value{ b } {
    DEBUG_PRINT("JsonVal => bool b");
}




Json& Json::operator=(JsonObject&& other) {
    m_value = std::make_unique<JsonObject>(std::move(other));
    DEBUG_PRINT("JsonVal operator => Json&& child");
    return *this;
}

Json& Json::operator=(const JsonObject& other) {
    m_value = std::make_unique<JsonObject>(other);
    DEBUG_PRINT("JsonVal operator => const Json& child");

    return *this;
}

Json & Json::operator=(const Array &child) {
    m_value = child;
    DEBUG_PRINT("JsonVal operator => const array& child");

    return *this;
}

Json & Json::operator=(Array &&child) {
    m_value = std::move(child);
    DEBUG_PRINT("JsonVal operator => const array& child");

    return *this;
}

Json& Json::operator=(Value&& newValue) noexcept {
    m_value = std::move(newValue);
    DEBUG_PRINT("JsonVal operator => Value&& newValue");

    return *this;
}

Json& Json::operator=(const Value& newValue) {
    m_value = CloneValue(newValue);
    DEBUG_PRINT("JsonVal operator => const Value& newValue");

    return *this;
}

Json& Json::operator=(Json&& other) noexcept {
    m_value = std::move(other.m_value);
    DEBUG_PRINT("JsonVal operator => JsonVal&& other");

    return *this;
}

Json& Json::operator=(const Json& other) {
    if (this == &other)
        return *this;

    m_value = CloneValue(other.m_value);
    DEBUG_PRINT("JsonVal operator => const JsonVal& other");

    return *this;
}

Json& Json::operator=(bool other) {
    m_value = other;
    return *this;
}


Json::operator Value&() { return m_value; }

Json::operator const Value&() const { return m_value; }


bool Json::operator==(const Json& other) const {
    return std::visit([&]<class T>(const T& val){
            if constexpr (std::same_as<T, Object>)
                return std::holds_alternative<T>(other.m_value) && *std::get<T>(other.m_value) == *val;
            else
                return std::holds_alternative<T>(other.m_value) && std::get<T>(other.m_value) == val;
        }, m_value);
}


#pragma region I hate to love c preprocessor
// This section is dedicated to this cursed amazing feature that deserves
// to be deprecated/context constrained 10 years ago

#define ADVANCE_IF_RAW(predicate, action) do {                             \
        const char* ptr{ input.data() };                                   \
        const char* end{ ptr + input.size() };                             \
        while (ptr != end && (predicate)) ++ptr; /* SIMD someday */        \
            if (ptr == end) [[unlikely]] action;                           \
            input.remove_prefix(static_cast<size_t>(ptr - input.data()));  \
    } while (0)

#define ADVANCE_IF(predicate) ADVANCE_IF_RAW(predicate, return false)

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

static bool DecodeUtf16(std::string_view& s, std::string& out) {
    constexpr auto hex{ [](const char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    } };

    auto readU{ [&](uint16_t& v) -> bool {
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
    } };

    uint16_t h;
    if (!readU(h)) return false;

    if (0xDC00 <= h && h <= 0xDFFF) return false; // lone low surrogate

    uint32_t code{ h };

    // surrogate?
    if (0xD800 <= h && h <= 0xDBFF) {
        if (s.empty() || s[0] != '\\') return false;
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


// SWAR things. I really need <simd> now.
static constexpr uint64_t kSwarOnes{ 0x0101010101010101ULL };
static constexpr uint64_t kSwarHigh{ 0x8080808080808080ULL };

static bool details_::ReadString(std::string_view& input, auto& val, const BufferInfo& info) {
    if (input.empty() || input.front() != '"')
        return false;

    const char* const start{ input.data() + 1 };
    const char* const end  { input.data() + input.size() };
    const char*       p    { start };

    bool     hasEscape{};
    uint64_t seen{};      // OR of every byte scanned. Bit 7 set means at least one non-ASCII byte.

    for (;;) {
        // Fast path: skip 8 bytes per step while none of them is '"', '\\' or a control char (< 0x20).
        while (end - p >= 8) {
            uint64_t w;
            std::memcpy(&w, p, sizeof w);

            const uint64_t quote    { w ^ (kSwarOnes * '"' ) };
            const uint64_t backslash{ w ^ (kSwarOnes * '\\') };
            const uint64_t special  { ((quote     - kSwarOnes) & ~quote    )
                                    | ((backslash - kSwarOnes) & ~backslash)
                                    | ((w - kSwarOnes * 0x20 ) & ~w        ) };
            const uint64_t hits{ special & kSwarHigh };
            if (hits) {
                // The lowest flagged byte is always a real special one (false positives only come after a real hit),
                // so jump straight to it and let the slow path below handle it.
                if constexpr (std::endian::native == std::endian::little) {
                    const unsigned idx{ static_cast<unsigned>(std::countr_zero(hits)) >> 3 };
                    seen |= w & ((uint64_t{ 1 } << (idx * 8)) - 1); // The clean bytes skipped, still counted for non-ASCII.
                    p += idx;
                }
                break;
            }

            seen |= w;
            p += 8;
        }

        // Slow path: one byte (also handles the tail of the buffer), then try the fast path again.
        if (p == end) [[unlikely]]
            return false;

        const auto c{ static_cast<unsigned char>(*p) };
        if (c == '"')
            break;

        if (c == '\\') {
            if (end - p < 2) [[unlikely]]
                return false;
            hasEscape = true;
            ++p; // Skip the escaped char, it will be validated by the decoder below. Every '\\' is followed by a char.
        } else if (c < 0x20) {
            return false;
        } else {
            seen |= c;
        }
        ++p;
    }

    const std::string_view raw{ start, static_cast<size_t>(p - start) };
    input.remove_prefix(static_cast<size_t>(p - input.data()) + 1);

    if ((seen & kSwarHigh) && !Thoth::String::Utf8View::IsValid(std::bit_cast<std::u8string_view>(raw)))
        return false;

    if (!hasEscape) {
        if constexpr (std::same_as<std::remove_cvref_t<decltype(val)>, std::string>)
            val = std::string{ raw };
        else
            val = String::FromRef({raw, info.buffer});
        return true;
    }

    std::string str;
    str.reserve(raw.size());

    std::string_view rest{ raw };
    for (size_t pos; (pos = rest.find('\\')) != std::string_view::npos; ) {
        str.append(rest.data(), pos);
        rest.remove_prefix(pos + 1);

        switch (rest.front()) { // Not empty: the scan above never leaves a '\\' as the last char.
            case 'u' : if (!DecodeUtf16(rest, str)) return false; continue; // It consumes its own chars.
            case '"' : str.push_back('"');  break;
            case '\\': str.push_back('\\'); break;
            case '/' : str.push_back('/');  break;
            case 'b' : str.push_back('\b'); break;
            case 'f' : str.push_back('\f'); break;
            case 'n' : str.push_back('\n'); break;
            case 'r' : str.push_back('\r'); break;
            case 't' : str.push_back('\t'); break;

            default: return false;
        }
        rest.remove_prefix(1);
    }
    str.append(rest.data(), rest.size());

    if constexpr (std::same_as<std::remove_cvref_t<decltype(val)>, std::string>)
        val = std::move(str);
    else
        val = String::FromOwned(std::move(str));
    return true;
}

// Exactly representable powers of ten (10^0 .. 10^22), for the fast double path.
static constexpr double kPow10[]{
    1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10, 1e11,
    1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22
};

static bool details_::ReadNumber(std::string_view& input, auto& val) {
    const char* const first{ input.data() };
    const char* const end  { first + input.size() };
    const char*       p    { first };

    const auto isDigit{ [](const char c) { return static_cast<unsigned>(c - '0') < 10u; } };

    const bool negative{ p != end && *p == '-' };
    if (negative)
        ++p;

    if (p == end || !isDigit(*p))
        return false;

    uint64_t mantissa{};
    int      digits{};

    if (*p == '0') {
        ++p;
        digits = 1;
    } else {
        do {
            mantissa = mantissa * 10 + static_cast<uint64_t>(*p - '0');
            ++digits;
            ++p;
        } while (p != end && isDigit(*p));
    }

    bool isFloat{};
    int  fracDigits{};
    if (p != end && *p == '.') {
        ++p;
        if (p == end || !isDigit(*p))
            return false;
        do {
            mantissa = mantissa * 10 + static_cast<uint64_t>(*p - '0');
            ++digits;
            ++fracDigits;
            ++p;
        } while (p != end && isDigit(*p));
        isFloat = true;
    }

    int exp10{};
    if (p != end && (*p == 'e' || *p == 'E')) {
        ++p;
        bool expNegative{};
        if (p != end && (*p == '+' || *p == '-')) {
            expNegative = *p == '-';
            ++p;
        }
        if (p == end || !isDigit(*p))
            return false;
        int e{};
        do {
            if (e < 100000) // Saturate, it only has to stay far outside the fast path range.
                e = e * 10 + (*p - '0');
            ++p;
        } while (p != end && isDigit(*p));
        exp10   = expNegative ? -e : e;
        isFloat = true;
    }

    // Anything number-like left here is malformed: "01", "1.2.3", "1-2", "1e5e"...
    if (p != end && (isDigit(*p) || *p == '.' || *p == '-' || *p == '+' || *p == 'e' || *p == 'E'))
        return false;

    const char* const last{ p };
    input.remove_prefix(static_cast<size_t>(last - first));

    const auto fromDouble{ [&] {
        double d;
        const auto [ptr, ec]{ std::from_chars(first, last, d) };
        if (ec != std::errc{} || ptr != last)
            return false;
        val = Number{ d };
        return true;
    } };

    if (isFloat) {
        const int e10{ exp10 - fracDigits };
        if (digits <= 19 && mantissa <= (1ULL << 53) && e10 >= -22 && e10 <= 22) {
            const double m{ static_cast<double>(mantissa) };
            const double d{ e10 < 0 ? m / kPow10[-e10] : m * kPow10[e10] };
            val = Number{ negative ? -d : d };
            return true;
        }
        return fromDouble();
    }

    if (negative) {
        if (digits <= 19 && mantissa <= (1ULL << 63)) {
            val = Number{ static_cast<int64_t>(0 - mantissa) };
            return true;
        }
        return fromDouble();
    }

    if (digits <= 19) {
        val = Number{ mantissa };
        return true;
    }

    uint64_t u;
    const auto [ptr, ec]{ std::from_chars(first, last, u) };
    if (ec == std::errc{} && ptr == last) {
        val = Number{ u };
        return true;
    }
    return fromDouble();
}

static bool details_::ReadObject(std::string_view& input, auto& val, const BufferInfo& info) {
    ADVANCE_SPACES();

    if (*input.data() != '{')
        return false;

    std::vector<std::pair<JsonObjKey, Json>> childs;
    childs.reserve(8);

    while (*input.data() != '}') {
        input.remove_prefix(1);

        ADVANCE_SPACES();

        if (childs.empty() && *input.data() == '}')
            break;

        std::string key;
        if (!ReadString(input, key, info))
            return false;

        ADVANCE_SPACES();

        if (*input.data() != ':')
            return false;
        input.remove_prefix(1);

        ADVANCE_SPACES();

        childs.emplace_back(std::move(key), NullJ);
        auto& newItem{ childs.back().second };

        bool success{};
        switch (*input.data()) {
            CASE_OPEN_STRING   success = ReadString(input, newItem, info); break;
            CASE_OPEN_NUMBER   success = ReadNumber(input, newItem);       break;
            CASE_OPEN_OBJECT   success = ReadObject(input, newItem, info); break;
            CASE_OPEN_BOOLEAN  success = ReadBool(  input, newItem);       break;
            CASE_OPEN_NULLABLE success = ReadNull(  input, newItem);       break;
            CASE_OPEN_ARRAY    success = ReadArray( input, newItem, info); break;
            default: return false;
        }
        if (!success)
            return false;

        ADVANCE_SPACES();

        if(*input.data() != ',' && *input.data() != '}')
            return false;
    }
    input.remove_prefix(1);

    val = JsonObject{ JsonObject::MapType{ std::move(childs) } };
    return true;
}
static bool details_::ReadBool(std::string_view& input, auto& val) {
    if (std::ranges::starts_with(input, std::string_view{ "true" }))
        input.remove_prefix(4), val = true;
    else if (std::ranges::starts_with(input, std::string_view{ "false" }))
        input.remove_prefix(5), val = false;
    else
        return false;
    return true;
}
static bool details_::ReadNull(std::string_view& input, auto& val) {
    if (std::ranges::starts_with(input, std::string_view{ "null" }))
        input.remove_prefix(4);
    else
        return false;

    val = NullV;
    return true;
}
static bool details_::ReadArray(std::string_view& input, auto& val, const BufferInfo& info) {
    if (*input.data() != '[')
        return false;

    Array array{};
    array.reserve(8);

    while (*input.data() != ']') {
        input.remove_prefix(1);
        ADVANCE_SPACES();

        if (array.empty() && *input.data() == ']')
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

        if (*input.data() != ',' && *input.data() != ']')
            return false;
    }
    input.remove_prefix(1);

    val = std::move(array);
    return true;
}

#pragma endregion


std::expected<Json, ThothError> Json::Parse(std::string_view input) {
    return ParseText(input);
}

std::expected<Json, ThothError> Json::ParseText(std::string_view input, bool copyData, bool checkFinal) {
    details_::BufferInfo info{};

    if (copyData) {
        info.buffer = std::make_shared<std::string>(input);
        info.bufferView = *info.buffer;
        input = info.bufferView;
    }
    else
        info.bufferView = input;

    const auto error{ [&]() -> std::unexpected<ThothError>{
        if (input.empty())
            return ThothUnex{ GenericError{ "Input for Json is empty" } };
        return ThothUnex{ JsonParseError{ info.bufferView.size() - input.size(), input[0] } };
    } };

#define return return error(); // I hate to do it
    ADVANCE_SPACES();
#undef return

    Json json{};
    bool success{};

    switch (input[0]){
        CASE_OPEN_STRING   success = details_::ReadString(input, json, info); break;
        CASE_OPEN_NUMBER   success = details_::ReadNumber(input, json);       break;
        CASE_OPEN_OBJECT   success = details_::ReadObject(input, json, info); break;
        CASE_OPEN_NULLABLE success = details_::ReadNull(  input, json);       break;
        CASE_OPEN_BOOLEAN  success = details_::ReadBool(  input, json);       break;
        CASE_OPEN_ARRAY    success = details_::ReadArray( input, json, info); break;
        default: return error();
    }
    if (!success)
        return error();

#define return json; // Oh god, no again
    if (checkFinal) {
        ADVANCE_SPACES();
    }
#undef return

    if (input.empty() || !checkFinal)
        return json;

    return error();
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


constexpr auto resolveKeys{ Hermes::Utils::Overloaded{
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
} };

#define RESOLVE_KEY_AND_RETURN(expected, error) \
        if (!std::visit([&](const auto& k){ return resolveKeys(curr, k); }, key)) \
            return error; \
        return expected;

#pragma region Get Functions

OptRefValWrapper Json::Get(const KeyView key) {
    OptRefValWrapper curr{ this };
    RESOLVE_KEY_AND_RETURN(curr, std::nullopt);
}
OptCRefValWrapper Json::Get(const KeyView key) const {
    OptCRefValWrapper curr{ this };
    RESOLVE_KEY_AND_RETURN(curr, std::nullopt);
}
OptValWrapper Json::GetCopy(const KeyView key) const {
    OptCRefValWrapper curr{ this };
    RESOLVE_KEY_AND_RETURN(*curr, std::nullopt);
}
OptValWrapper Json::GetAndMove(const KeyView key) && {
    OptRefValWrapper curr{ this };
    RESOLVE_KEY_AND_RETURN(std::move(**curr), std::nullopt);
}


ValWrapper Json::GetCopyOrNull(const KeyView key) const {
    OptCRefValWrapper curr{ this };
    RESOLVE_KEY_AND_RETURN(**curr, NullJ);
}
ValWrapper Json::GetAndMoveOrNull(const KeyView key) && {
    OptRefValWrapper curr{ this };
    RESOLVE_KEY_AND_RETURN(std::move(**curr), NullJ);
}


#define KEY_NOT_FOUND ThothUnex{ JsonGetError{ Key{ key } } }

ExpRefValWrapper Json::GetOrError(const KeyView key) {
    OptRefValWrapper curr{ this };
    RESOLVE_KEY_AND_RETURN(*curr, KEY_NOT_FOUND);
}
ExpCRefValWrapper Json::GetOrError(const KeyView key) const {
    OptCRefValWrapper curr{ this };
    RESOLVE_KEY_AND_RETURN(*curr, KEY_NOT_FOUND);
}
ExpValWrapper Json::GetCopyOrError(const KeyView key) const {
    OptCRefValWrapper curr{ this };
    RESOLVE_KEY_AND_RETURN(**curr, KEY_NOT_FOUND);
}
ExpValWrapper Json::GetAndMoveOrError(const KeyView key) && {
    OptRefValWrapper curr{ this };
    RESOLVE_KEY_AND_RETURN(std::move(**curr), KEY_NOT_FOUND);
}

#undef KEY_NOT_FOUND
#pragma endregion

#pragma region Find Functions

OptRefValWrapper Json::Find(const KeysView keys) {
    OptRefValWrapper curr{ this };
    for (const auto& key : keys)
        RESOLVE_KEY_AND_RETURN(curr, std::nullopt);
}
OptCRefValWrapper Json::Find(const KeysView keys) const {
    OptCRefValWrapper curr{ this };
    for (const auto& key : keys)
        RESOLVE_KEY_AND_RETURN(curr, std::nullopt);
}
OptValWrapper Json::FindCopy(const KeysView keys) const {
    OptCRefValWrapper curr{ this };
    for (const auto& key : keys)
        RESOLVE_KEY_AND_RETURN(**curr, std::nullopt);
}
OptValWrapper Json::FindAndMove(const KeysView keys) && {
    OptRefValWrapper curr{ this };
    for (const auto& key : keys)
        RESOLVE_KEY_AND_RETURN(std::move(**curr), std::nullopt);
}


ValWrapper Json::FindCopyOrNull(const KeysView keys) const {
    OptCRefValWrapper curr{ this };
    for (const auto& key : keys)
        RESOLVE_KEY_AND_RETURN(**curr, NullJ);
}
ValWrapper Json::FindAndMoveOrNull(const KeysView keys) && {
    OptRefValWrapper curr{ this };
    for (const auto& key : keys)
        RESOLVE_KEY_AND_RETURN(std::move(**curr), NullJ);
}


#define KEY_NOT_FOUND ThothUnex{ JsonFindError{ \
    Key{ key }, \
    keys | std::views::transform([](const auto& k) { return Key{ k }; }) | std::ranges::to<std::vector>() \
} }

ExpRefValWrapper Json::FindOrError(const KeysView keys) {
    OptRefValWrapper curr{ this };
    auto sla{ keys | std::ranges::to<std::vector>() };
    for (const auto& key : keys)
        RESOLVE_KEY_AND_RETURN(*curr, KEY_NOT_FOUND);
}

ExpCRefValWrapper Json::FindOrError(const KeysView keys) const {
    OptCRefValWrapper curr{ this };
    for (const auto& key : keys)
        RESOLVE_KEY_AND_RETURN(*curr, KEY_NOT_FOUND);
}

ExpValWrapper Json::FindCopyOrError(const KeysView keys) const {
    OptCRefValWrapper curr{ this };
    for (const auto& key : keys)
        RESOLVE_KEY_AND_RETURN(**curr, KEY_NOT_FOUND);
}

ExpValWrapper Json::FindAndMoveOrError(const KeysView keys) && {
    OptRefValWrapper curr{ this };
    for (const auto& key : keys)
        RESOLVE_KEY_AND_RETURN(std::move(**curr), KEY_NOT_FOUND);
}


#pragma endregion
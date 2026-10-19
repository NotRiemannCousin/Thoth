// ReSharper disable CppPassValueParameterByConstReference

#include <algorithm>
#include <execution>
#include <expected>
#include <cstdint>
#include <cstring>
#include <bit>
#include <optional>

#include <Thoth/String/UnicodeViewer.hpp>
#include <Thoth/ThothError.hpp>
#include <Thoth/Utils/Functional.hpp>
#include <Thoth/NJson/Json.hpp>
#include "fast_float.h"

using namespace Thoth::NJson;
using namespace Thoth::Http;
using Thoth::ThothError;

#ifdef DENSE_DEBUG_JSON
#include <print>
#define DEBUG_PRINT(MSG) std::println(MSG);
#else
#define DEBUG_PRINT(MSG)
#endif

namespace {
    // Everything here seams to be claiming to get UB but it works.

    bool DecodeUtf16(std::string_view& s, std::string& out) {
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

        if (code < 0x80) {
            out.push_back(static_cast<char>(code));
        } else if (code < 0x800) {
            out.push_back(static_cast<char>(0xC0 | (code>>6)));
            out.push_back(static_cast<char>(0x80 | (code&63)));
        } else if (code < 0x10000) {
            out.push_back(static_cast<char>(0xE0 | (code>>12)));
            out.push_back(static_cast<char>(0x80 | ((code>>6)&63)));
            out.push_back(static_cast<char>(0x80 | (code&63)));
        } else {
            out.push_back(static_cast<char>(0xF0 | (code>>18)));
            out.push_back(static_cast<char>(0x80 | ((code>>12)&63)));
            out.push_back(static_cast<char>(0x80 | ((code>>6)&63)));
            out.push_back(static_cast<char>(0x80 | (code&63)));
        }

        return true;
    }

    // SWAR things. I really need <simd> now.
    constexpr uint64_t kSwarOnes{ 0x0101010101010101ULL };
    constexpr uint64_t kSwarHigh{ 0x8080808080808080ULL };



    // Johnny Lee's algorithm — parse 8 ASCII digits into a uint32.
    // `chars` MUST point to 8 bytes that are all ASCII digits (caller validates).
    // Reference: https://johnnylee.io/2016/04/24/simd-parsing-of-integers/
    uint32_t ParseEightDigits(const char* chars) noexcept {
        uint64_t val;
        std::memcpy(&val, chars, 8);
        val = (val & 0x0F0F0F0F0F0F0F0FULL) * 2561ULL               // 0x0A01
                >> 8;
        val = (val & 0x00FF00FF00FF00FFULL) * 6553601ULL             // 0x00640001
                >> 16;
        val = (val & 0x0000FFFF0000FFFFULL) * 42949672960001ULL      // 0x0000000100002710
                >> 32;
        return static_cast<uint32_t>(val);
    }

    // SWAR: are all 8 bytes at `chars` ASCII digits (0x30..0x39)?
    //   - High nibble must be 3  ⇒ byte ∈ [0x30, 0x3F].
    //   - Low nibble + 6 must not carry into bit 4 ⇒ low nibble ∈ [0..9]. Each byte of `low` ≤ 0x0F, so
    //   0x0F + 0x06 = 0x15 < 0x100, no carry across bytes — meaning the per-byte carry is observable in bit 4.
    bool AreEightDigits(const char* chars) noexcept {
        uint64_t chunk;
        std::memcpy(&chunk, chars, 8);

        if ((chunk & 0xF0F0F0F0F0F0F0F0ULL) != 0x3030303030303030ULL)
            return false;

        const uint64_t low{ chunk & 0x0F0F0F0F0F0F0F0FULL };
        if ((low + 0x0606060606060606ULL) & 0xF0F0F0F0F0F0F0F0ULL)
            return false;

        return true;
    }

    // Append ASCII digits from [p, end) into `mantissa` (base 10), counting how many were consumed in `digits`.
    // Stops at the first non-digit or at end.
    //
    // On uint64 overflow, `mantissa` saturates to UINT64_MAX (callers detect overflow via `digits > 19 &&
    // mantissa == UINT64_MAX`); digit counting continues so the caller still knows the literal length.
    void AccumulateDigits(uint64_t& mantissa, int& digits,
                                 const char*& p, const char* end) noexcept {
        bool saturated{ mantissa == std::numeric_limits<uint64_t>::max() };

        // SWAR: 8 digits at a time, while:
        //   - mantissa * 10^8 + val8 fits in uint64  ⇒  mantissa < 10^11  (since 10^11 * 10^8 = 10^19 < 2^64, and
        //   val8 < 10^8)
        //   - digits + 8 <= 19  (uint64 holds at most ~1.8e19, 19 digits)
        while (!saturated && end - p >= 8 && digits + 8 <= 19 &&
               mantissa < 100000000000ULL) {  // 10^11
            if (!AreEightDigits(p))
                break;

            const uint32_t val8{ ParseEightDigits(p) };
            mantissa = mantissa * 100000000ULL + val8;  // mantissa * 10^8 + val8
            digits  += 8;
            p       += 8;
        }

       while (p != end) {
            const unsigned c{ static_cast<unsigned>(*p) - '0' };
            if (c >= 10u)
                break;

            if (!saturated) {
                // mantissa * 10 + c, with overflow check.
                // UINT64_MAX = 18446744073709551615.
                if (mantissa < 1844674407370955161ULL) {
                    mantissa = mantissa * 10 + c;
                } else if (mantissa == 1844674407370955161ULL && c <= 5) {
                    mantissa = mantissa * 10 + c;
                } else {
                    mantissa  = UINT64_MAX;  // saturate
                    saturated = true;
                }
            }
            ++digits;
            ++p;
        }
    }

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

bool details_::ReadString(std::string_view& input, auto& val, const BufferInfo& info) {
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
bool details_::ReadNumber(std::string_view& input, auto& val) {
    const char* const first{ input.data() };
    const char* const end  { first + input.size() };
    const char*       p    { first };

    const bool negative{ p != end && *p == '-' };
    if (negative)
        ++p;

    if (p == end || static_cast<unsigned>(*p - '0') >= 10u)
        return false;

    uint64_t mantissa{};
    int      digits{};

    // Integer part. Leading '0' is allowed only as a single digit (no "01").
    if (*p == '0') {
        ++p;
        digits = 1;
    } else AccumulateDigits(mantissa, digits, p, end);

    bool isFloat{};
    int  fracDigits{};

    // Fractional part.
    if (p != end && *p == '.') {
        ++p;
        if (p == end || static_cast<unsigned>(*p - '0') >= 10u)
            return false;

        const int digitsBefore = digits;
        AccumulateDigits(mantissa, digits, p, end);
        fracDigits = digits - digitsBefore;
        isFloat = true;
    }

    // Exponent part.
    int exp10{};
    if (p != end && (*p == 'e' || *p == 'E')) {
        ++p;
        bool expNegative{};
        if (p != end && (*p == '+' || *p == '-')) {
            expNegative = *p == '-';
            ++p;
        }
        if (p == end || static_cast<unsigned>(*p - '0') >= 10u)
            return false;
        int e{};
        do {
            if (e < 100000) // Saturate; only needs to stay out of fast-path range.
                e = e * 10 + (*p - '0');
            ++p;
        } while (p != end && static_cast<unsigned>(*p - '0') < 10u);
        exp10   = expNegative ? -e : e;
        isFloat = true;
    }

    // Anything number-like left here is malformed: "01", "1.2.3", "1-2", "1e5e"...
    if (p != end) {
        const char c = *p;
        if (static_cast<unsigned>(c - '0') < 10u ||
            c == '.' || c == '-' || c == '+' || c == 'e' || c == 'E')
            return false;
    }

    input.remove_prefix(static_cast<size_t>(p - first));

    // ---- Type decision (unified; no double-parse) ----
    if (!isFloat) {
        // Pure integer literal — pick the smallest fitting integer type,
        // falling back to double on overflow.
        if (!negative) {
            if (digits <= 19) {
                if (mantissa <= static_cast<uint64_t>(INT64_MAX)) {
                    val = Number{ static_cast<int64_t>(mantissa) };
                    return true;
                }
                val = Number{ mantissa };  // uint64_t
                return true;
            }
            // digits > 19 — saturate-overflowed uint64. Fall through to double.
        } else {
            // Negative integer.
            if (digits <= 18 ||
                (digits == 19 && mantissa <= (1ULL << 63))) {
                val = Number{ -static_cast<int64_t>(mantissa) };
                return true;
            }
            if (digits == 19 && mantissa == (1ULL << 63)) {
                val = Number{ INT64_MIN };
                return true;
            }
            // Out of int64 range — fall through to double.
        }
    }

    // ---- Float path: fast_float::integer_times_pow10 ----
    //
    // fast_float takes (mantissa, decimal_exponent) and returns the
    // correctly-rounded double. Internally it runs:
    //   1. Clinger's fast path  — for mantissa < 2^53, |e10| <= 22 (exact).
    //   2. Eisel-Lemire          — for everything in normal double range.
    //   3. digit_comp fallback   — subnormals and edge cases (rare).
    //
    // Crucially: fast_float uses the mantissa WE already accumulated. It
    // does NOT re-parse the literal from the buffer. That's the win.
    const int e10{ exp10 - fracDigits };
    const double d{ fast_float::integer_times_pow10(mantissa, e10) };
    val = Number{ negative ? -d : d };
    return true;
}
bool details_::ReadObject(std::string_view& input, auto& val, const BufferInfo& info) {
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
bool details_::ReadBool(std::string_view& input, auto& val) {
    if (std::ranges::starts_with(input, std::string_view{ "true" }))
        input.remove_prefix(4), val = true;
    else if (std::ranges::starts_with(input, std::string_view{ "false" }))
        input.remove_prefix(5), val = false;
    else
        return false;
    return true;
}
bool details_::ReadNull(std::string_view& input, auto& val) {
    if (std::ranges::starts_with(input, std::string_view{ "null" }))
        input.remove_prefix(4);
    else
        return false;

    val = NullV;
    return true;
}
bool details_::ReadArray(std::string_view& input, auto& val, const BufferInfo& info) {
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
        auto buffer{ std::make_shared_for_overwrite<char[]>(input.size()) };
        if (!input.empty())
            std::memcpy(buffer.get(), input.data(), input.size());
        info.buffer = std::shared_ptr<const char[]>{ std::move(buffer) }; // char[] -> const char[]
        info.bufferView = std::string_view{ info.buffer.get(), input.size() };
        input = info.bufferView;
    } else {
        info.buffer = std::shared_ptr<const char[]>{ input.data(), [](const char*) noexcept {} };
        info.bufferView = input;
    }

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

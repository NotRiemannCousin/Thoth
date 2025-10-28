//
// Created by MARCE on 18/10/2025.
//

#include <algorithm>
#include <expected>
#include <functional>
#include <ranges>
#include <Thoth/Json/Json.hpp>

#include <Thoth/Http/Request/HttpUrl.hpp>

#include <Thoth/String/UnicodeView.hpp>

using namespace Thoth::Json;


#pragma region JsonVal

CONSTEXPR_WHEN_MSVC_STARTS_WORKING Json::JsonVal::JsonVal(const Json &child) : value{ std::make_unique<Json>(child) } { }

CONSTEXPR_WHEN_MSVC_STARTS_WORKING Json::JsonVal::JsonVal(Json &&child)  : value{ std::make_unique<Json>(child) } { }

CONSTEXPR_WHEN_MSVC_STARTS_WORKING Json::JsonVal::JsonVal(Value &&newValue) : value{std::move(newValue)} { }

CONSTEXPR_WHEN_MSVC_STARTS_WORKING Json::JsonVal::JsonVal(const Value& newValue) {
    auto res { std::visit([]<class T>(this auto const& self, const T& value){
        using Type = std::remove_cvref_t<T>;

        if constexpr (std::same_as<Type, std::unique_ptr<Json>>)
            return Value{ std::move(std::make_unique<Json>(*value)) };
        else
            return Value{ value };

    }, newValue) };
    value = std::move(res);
}

CONSTEXPR_WHEN_MSVC_STARTS_WORKING Json::JsonVal::JsonVal(JsonVal &&other) noexcept {
    value = std::move(other.value);
}

CONSTEXPR_WHEN_MSVC_STARTS_WORKING Json::JsonVal::JsonVal(const JsonVal& other) : JsonVal(other.value) { }

CONSTEXPR_WHEN_MSVC_STARTS_WORKING Json::JsonVal& Json::JsonVal::operator=(JsonVal && other) noexcept {
    value = std::move(other.value);
    return *this;
}

CONSTEXPR_WHEN_MSVC_STARTS_WORKING Json::JsonVal& Json::JsonVal::operator=(const JsonVal &other) {
    auto res { std::visit([]<class T>(this auto const& self, const T& value){
        using Type = std::remove_cvref_t<T>;

        if constexpr (std::same_as<Type, std::unique_ptr<Json>>)
            return Value{ std::move(std::make_unique<Json>(*value)) };
        else
            return Value{ value };

    }, other.value) };
    value = std::move(res);


    return *this;
}

CONSTEXPR_WHEN_MSVC_STARTS_WORKING Json::JsonVal::operator Value&() {
    return value;
}

CONSTEXPR_WHEN_MSVC_STARTS_WORKING Json::JsonVal::operator const Value&() const {
    return value;
}

#pragma endregion


#pragma region Json

CONSTEXPR_WHEN_MSVC_STARTS_WORKING Json::Json(MapType&& initAs) : pairs{ std::move(initAs) } { }

CONSTEXPR_WHEN_MSVC_STARTS_WORKING Json::Json(const std::initializer_list<JsonPair> &init) : pairs(init) { }

constexpr bool IsWhitespace(const char c) {
    return c == ' '
            || c == '\t'
            || c == '\n'
            || c == '\r';
}

constexpr bool IsNumber(const char c) {
    return (c >= '0' && c <= '9')
            || c == '.'
            || c == '-'
            || c == 'e'
            || c == 'E';
};


std::optional<Json> Json::Parse(std::string_view text) {
    if (!Thoth::String::Utf8View::IsValid(bit_cast<std::u8string_view>(text)))
        return std::nullopt;

    auto it{ text.cbegin() };

    return ParseUnchecked(it, text.cend()).
            and_then([&](const auto& json) -> std::optional<Json> {
                return std::all_of(it, text.end(), IsWhitespace)
                        ? std::optional{ json }
                        :  std::nullopt;
            });
}



#define CHECK_NULLOPT(EXTRA)     \
    if (it == itEnd || (EXTRA))  \
        return std::nullopt;     \

std::optional<Json> Json::ParseUnchecked(
    std::string_view::const_iterator &it,
    const std::string_view::const_iterator& itEnd
    ) {

    char last{};
    const auto ClosingKey = [&](auto c) {
        if (last != '\\' && c == '"')
            return false;

        last = c;
        return true;
    };

    const auto MatchIfNot = [&](auto pred) {
        return std::find_if_not(it, itEnd, pred);
    };


     it = MatchIfNot(IsWhitespace);
    CHECK_NULLOPT(*it != '{');
    Json json;

#pragma region I hate to love c preprocessor

#define CASE_OPEN_STRING   case '"':
#define CASE_OPEN_NUMBER   case '0':case '1':case '2':case '3':case '4':case '5':case '6':case '7':case '8':case '9':case '-':
#define CASE_OPEN_OBJECT   case '{':
#define CASE_OPEN_BOOLEAN  case 'f':case 't':
#define CASE_OPEN_NULLABLE case 'n':
#define CASE_OPEN_ARRAY    case '[':


    std::function<std::optional<JsonVal>()> ReadString, ReadNumber, ReadObject, ReadBoolean,
            ReadNullable, ReadArray;

    ReadString   = [&]() -> std::optional<JsonVal> {
        const auto openValStr{ it };
        CHECK_NULLOPT(*it++ != '"');

        const auto closeValStr{ it = MatchIfNot(ClosingKey) };
        CHECK_NULLOPT(*it++ != '"');

        return std::string{ openValStr + 1, closeValStr };
    };
    ReadNumber   = [&]() -> std::optional<JsonVal> {
        const auto openValNumber{ &*it };

        it = MatchIfNot(IsNumber);
        const auto closeValNumber{ &*it };
        CHECK_NULLOPT(false);

        long double number{};

        auto [ptr, err] {
            std::from_chars(
                    &*openValNumber,
                    &*closeValNumber,
                    number
                )};
        if (err != std::errc{} || ptr != closeValNumber)
            return std::nullopt;

        return number;
    };
    ReadObject   = [&]() -> std::optional<JsonVal> {
        const auto res{ ParseUnchecked(it, itEnd) };
        if (!res)
            return std::nullopt;

        CHECK_NULLOPT(false);
        return JsonVal{ *res };
    };
    ReadBoolean  = [&]() -> std::optional<JsonVal> {
        auto view{ std::string_view{ it, itEnd } };
        bool val{};

        if (std::ranges::starts_with(view, std::string_view{ "true" }))
            it += 4, val = true;
        else if (std::ranges::starts_with(view, std::string_view{ "false" }))
            it += 5, val = false;
        else
            return std::nullopt;

        CHECK_NULLOPT(false);
        return JsonVal::Value{ val };
    };
    ReadNullable = [&]() -> std::optional<JsonVal> {
        const auto view{ std::string_view{ it, itEnd } };
        if (std::ranges::starts_with(view, std::string_view{ "null" }))
            it += 4;
        else
            return std::nullopt;

        CHECK_NULLOPT(false);
        return NullV;
    };
    ReadArray    = [&]() -> std::optional<JsonVal> {
        Array array{};

        static_assert(std::same_as<decltype(*it), const char&>);
        while (*it != ']') {
            ++it;

            it = MatchIfNot(IsWhitespace);
            CHECK_NULLOPT(false);

            std::optional<JsonVal> value;
            switch (*it) {
                CASE_OPEN_STRING   value = ReadString();   break;
                CASE_OPEN_NUMBER   value = ReadNumber();   break;
                CASE_OPEN_OBJECT   value = ReadObject();   break;
                CASE_OPEN_BOOLEAN  value = ReadBoolean();  break;
                CASE_OPEN_NULLABLE value = ReadNullable(); break;
                CASE_OPEN_ARRAY    value = ReadArray();    break;
                default: return std::nullopt;
            }
            if (!value)
                return std::nullopt;

            array.emplace_back(*value);

            it = MatchIfNot(IsWhitespace);
            CHECK_NULLOPT(*it != ',' && *it != ']');
        }

        ++it;
        CHECK_NULLOPT(false);
        return array;
    };

#pragma endregion

    while (*it != '}') {
        ++it;

        const auto openKey{ it = MatchIfNot(IsWhitespace) };
        CHECK_NULLOPT(*it++ != '"');
        const auto closeKey{ it = MatchIfNot(ClosingKey) };
        CHECK_NULLOPT(*it++ != '"');
        const std::string_view key{ openKey + 1, closeKey };

        it = MatchIfNot(IsWhitespace);
        CHECK_NULLOPT(*it++ != ':');

        it = MatchIfNot(IsWhitespace);
        CHECK_NULLOPT(false);

        std::optional<JsonVal> value;
        switch (*it) {
            CASE_OPEN_STRING   value = ReadString();   break;
            CASE_OPEN_NUMBER   value = ReadNumber();   break;
            CASE_OPEN_OBJECT   value = ReadObject();   break;
            CASE_OPEN_BOOLEAN  value = ReadBoolean();  break;
            CASE_OPEN_NULLABLE value = ReadNullable(); break;
            CASE_OPEN_ARRAY    value = ReadArray();    break;
            default: return std::nullopt;
        }
        if (!value)
            return std::nullopt;

        json.pairs.emplace(key, *value);

        it = MatchIfNot(IsWhitespace);
        CHECK_NULLOPT(*it != ',' && *it != '}');
    }

    ++it;
    return json;
}

#undef CHECK_NULLOPT

bool Json::Exists(JsonKeyRef key) const {
    return pairs.contains(key);
}

bool Json::Exists(JsonPairRef p) const {
    return Exists(p.first, p.second);
}

bool Json::Exists(JsonKeyRef key, JsonValRef val) const {
    const auto it{ pairs.find(key) };

    return it != pairs.end() && it->second == val;
}

void Json::Set(JsonPairRef p) {
    Set(p.first, p.second);
}

void Json::Set(JsonKeyRef key, JsonValRef val) {
    if (!Exists(key)) return;

    pairs.emplace(key, val);
}

bool Json::Remove(JsonKeyRef key) {
    return pairs.erase(key);
}


bool Json::SetIfNull(JsonPairRef p) {
    return SetIfNull(p.first, p.second);
}

bool Json::SetIfNull(JsonKeyRef key, JsonValRef val) {
    auto [_, tookPlace]{ pairs.emplace(key, val) };

    return tookPlace;
}

std::optional<std::reference_wrapper<Json::JsonVal>> Json::Get(JsonKeyRef key) {
    const auto it{ pairs.find(key) };

    if (it == pairs.end())
        return std::nullopt;
    return it->second;
}

std::optional<std::reference_wrapper<const Json::JsonVal>> Json::Get(JsonKeyRef key) const {
    const auto it{ pairs.find(key) };

    if (it == pairs.end())
        return std::nullopt;
    return it->second;
}

std::expected<std::reference_wrapper<Json::JsonVal>, Null> Json::GetOrNull(JsonKeyRef key) {
    const auto it{ pairs.find(key) };

    if (it == pairs.end())
        return std::unexpected{ NullV };
    return it->second;
}

std::expected<std::reference_wrapper<const Json::JsonVal>, Null> Json::GetOrNull(JsonKeyRef key) const {
    const auto it{ pairs.find(key) };

    if (it == pairs.end())
        return std::unexpected{ NullV };
    return it->second;
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
    const auto [it, _]{ pairs.emplace(key, NullV) };

    return it->second;
}

const Json::JsonVal& Json::operator[](JsonKeyRef key) const {
    const auto it{ pairs.find(key) };
    return it != pairs.end() ? it->second : NullJ;
}

#pragma endregion

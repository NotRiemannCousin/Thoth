// ReSharper disable CppPassValueParameterByConstReference

#include <functional>
#include <algorithm>
#include <expected>
#include <print>

#include <Thoth/NJson/Json.hpp>



#ifdef DENSE_DEBUG_JSON
#include <print>
#define DEBUG_PRINT(MSG) std::println(MSG);
#else
#define DEBUG_PRINT(MSG)
#endif

using namespace Thoth::NJson;

JsonObject::~JsonObject() {
    DEBUG_PRINT("~Json destructor");
}

JsonObject::JsonObject(const JsonObject& other) {
    DEBUG_PRINT("JsonObject => const JsonVal& other");
    _pairs = other._pairs;
    // If the buffer isn't user managed carries the reference, so when you delete the parent but not the children the
    // Cow still works. Yeah, given the necessity of `_bufferView` (user managed buffers) the `_buffer` itself it's
    // kinda dummy, it just holds the reference.
}

JsonObject::JsonObject(JsonObject&& other) noexcept : _pairs{ std::move(other._pairs) } {
    DEBUG_PRINT("JsonObject => JsonVal&& other");
}

JsonObject::JsonObject(MapType&& initAs) : _pairs{ std::move(initAs) } { }

JsonObject::JsonObject(std::initializer_list<JsonPair> init) : _pairs{ init } {
    DEBUG_PRINT("JsonObject initializer_list");
}



JsonObject& JsonObject::operator=(const JsonObject& other) {
    if (this == &other)
        return *this;

    DEBUG_PRINT("JsonObject equals operator => const JsonVal& other");
    _pairs = other._pairs;

    return *this;
}

JsonObject & JsonObject::operator=(std::initializer_list<JsonPair> list) {
    _pairs = list;
    DEBUG_PRINT("JsonObject equals operator => initializer_list");
    return *this;
}

JsonObject& JsonObject::operator=(JsonObject&& other) noexcept {
    _pairs = std::move(other._pairs);
    DEBUG_PRINT("JsonObject equals operator => const JsonVal& other");
    return *this;
}


bool JsonObject::Exists(JsonObjKeyRef key) const {
    return _pairs.contains(key);
}

bool JsonObject::Exists(JsonPairRef p) const {
    return Exists(p.first, p.second);
}

bool JsonObject::Exists(JsonObjKeyRef key, JsonValRef val) const {
    const auto it{ _pairs.find(key) };

    return it != _pairs.end()&& it->second == val;
}

void JsonObject::Set(JsonPairRef p) {
    Set(p.first, p.second);
}

void JsonObject::Set(JsonObjKeyRef key, JsonValRef val) {
    _pairs.try_emplace(key, val);
}

bool JsonObject::Remove(JsonObjKeyRef key) {
    return _pairs.erase(key);
}


bool JsonObject::SetIfNull(JsonPairRef p) {
    return SetIfNull(p.first, p.second);
}

bool JsonObject::SetIfNull(JsonObjKeyRef key, JsonValRef val) {
    auto [_, tookPlace]{ _pairs.try_emplace(key, val) };

    return tookPlace;
}


OptRefValWrapper JsonObject::Get(JsonObjKeyRef key) {
    if (const auto it{ _pairs.find(key) }; it != _pairs.end())
        return it->second;
    return std::nullopt;
}

OptCRefValWrapper JsonObject::Get(JsonObjKeyRef key) const {
    if (const auto it{ _pairs.find(key) }; it != _pairs.end())
        return it->second;
    return std::nullopt;
}

CRefValWrapperOrNull JsonObject::GetOrNull(JsonObjKeyRef key) const {
    return Get(key).value_or(NullJ);
}


OptValWrapper JsonObject::GetCopy(JsonObjKeyRef key) const {
    if (const auto it{ _pairs.find(key) }; it != _pairs.end())
        return it->second;
    return std::nullopt;
}

ValWrapperOrNull JsonObject::GetOrNullCopy(JsonObjKeyRef key) const {
    return GetCopy(key).value_or(NullJ);
}


OptValWrapper JsonObject::GetAndMove(JsonObjKeyRef key) && {
    if (const auto it{ _pairs.find(key) }; it != _pairs.end())
        return std::move(it->second);
    return std::nullopt;
}

ValWrapperOrNull JsonObject::GetOrNullAndMove(JsonObjKeyRef key) && {
    if (const auto it{ _pairs.find(key) }; it != _pairs.end())
        return std::move(it->second);
    return NullJ;
}


void JsonObject::Clear() {
    _pairs.clear();
}

size_t JsonObject::Size() const {
    return _pairs.size();
}

bool JsonObject::Empty() const {
    return _pairs.empty();
}

Json& JsonObject::operator[](JsonObjKeyRef key) {
    const auto [it, _]{ _pairs.try_emplace(key, NullV) };

    return it->second;
}

const Json& JsonObject::operator[](JsonObjKeyRef key) const {
    const auto it{ _pairs.find(key) };
    return it != _pairs.end() ? it->second : NullJ;
}

bool JsonObject::operator==(const JsonObject& other) const {
    return _pairs == other._pairs;
}

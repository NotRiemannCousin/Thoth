// ReSharper disable CppPassValueParameterByConstReference

#include <algorithm>
#include <expected>

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
    m_pairs = other.m_pairs;
    // If the buffer isn't user managed carries the reference, so when you delete the parent but not the children the
    // Cow still works. Yeah, given the necessity of `m_bufferView` (user managed buffers) the `m_buffer` itself it's
    // kinda dummy, it just holds the reference.
}

JsonObject::JsonObject(JsonObject&& other) noexcept : m_pairs{ std::move(other.m_pairs) } {
    DEBUG_PRINT("JsonObject => JsonVal&& other");
}

JsonObject::JsonObject(MapType&& initAs) : m_pairs{ std::move(initAs) } { }

JsonObject::JsonObject(std::initializer_list<JsonPair> init) : m_pairs{ init } {
    DEBUG_PRINT("JsonObject initializer_list");
}



JsonObject& JsonObject::operator=(const JsonObject& other) {
    if (this == &other)
        return *this;

    DEBUG_PRINT("JsonObject equals operator => const JsonVal& other");
    m_pairs = other.m_pairs;

    return *this;
}

JsonObject & JsonObject::operator=(std::initializer_list<JsonPair> list) {
    m_pairs = list;
    DEBUG_PRINT("JsonObject equals operator => initializer_list");
    return *this;
}

JsonObject& JsonObject::operator=(JsonObject&& other) noexcept {
    m_pairs = std::move(other.m_pairs);
    DEBUG_PRINT("JsonObject equals operator => const JsonVal& other");
    return *this;
}


bool JsonObject::Exists(JsonObjKeyRef key) const {
    return m_pairs.contains(key);
}

bool JsonObject::Exists(JsonPairRef p) const {
    return Exists(p.first, p.second);
}

bool JsonObject::Exists(JsonObjKeyRef key, JsonValRef val) const {
    const auto it{ m_pairs.find(key) };

    return it != m_pairs.end()&& it->second == val;
}

void JsonObject::Set(JsonPairRef p) {
    Set(p.first, p.second);
}

void JsonObject::Set(JsonObjKeyRef key, JsonValRef val) {
    m_pairs.insert_or_assign(key, val);
}

bool JsonObject::Remove(JsonObjKeyRef key) {
    return m_pairs.erase(key);
}


bool JsonObject::SetIfNull(JsonPairRef p) {
    return SetIfNull(p.first, p.second);
}

bool JsonObject::SetIfNull(JsonObjKeyRef key, JsonValRef val) {
    auto [_, tookPlace]{ m_pairs.try_emplace(key, val) };

    return tookPlace;
}


OptRefValWrapper JsonObject::Get(JsonObjKeyRef key) {
    if (const auto it{ m_pairs.find(key) }; it != m_pairs.end())
        return &it->second;
    return std::nullopt;
}

OptCRefValWrapper JsonObject::Get(JsonObjKeyRef key) const {
    if (const auto it{ m_pairs.find(key) }; it != m_pairs.end())
        return &it->second;
    return std::nullopt;
}

OptValWrapper JsonObject::GetCopy(JsonObjKeyRef key) const {
    if (const auto it{ m_pairs.find(key) }; it != m_pairs.end())
        return it->second;
    return std::nullopt;
}

ValWrapper JsonObject::GetCopyOrNull(JsonObjKeyRef key) const {
    return GetCopy(key).value_or(NullJ);
}


OptValWrapper JsonObject::GetAndMove(JsonObjKeyRef key) && {
    if (const auto it{ m_pairs.find(key) }; it != m_pairs.end())
        return std::move(it->second);
    return std::nullopt;
}

ValWrapper JsonObject::GetOrNullAndMove(JsonObjKeyRef key) && {
    if (const auto it{ m_pairs.find(key) }; it != m_pairs.end())
        return std::move(it->second);
    return NullJ;
}


void JsonObject::Clear() {
    m_pairs.clear();
}

size_t JsonObject::Size() const {
    return m_pairs.size();
}

bool JsonObject::Empty() const {
    return m_pairs.empty();
}

Json& JsonObject::operator[](JsonObjKeyRef key) {
    const auto [it, _]{ m_pairs.try_emplace(key, NullV) };

    return it->second;
}

const Json& JsonObject::operator[](JsonObjKeyRef key) const {
    const auto it{ m_pairs.find(key) };
    return it != m_pairs.end() ? it->second : NullJ;
}

bool JsonObject::operator==(const JsonObject& other) const {
    return m_pairs == other.m_pairs;
}

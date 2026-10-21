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

#pragma region Assignment


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


#pragma endregion


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

#pragma region Range Interface

    Json::iterator Json::begin() {
        if (auto obj{ EnsureMut<Object>() })
            return iterator{ (**obj)->begin() };
        if (auto arr{ EnsureMut<Array>() })
            return iterator{ (*arr)->begin(), 0 };

        return {};
    }

    Json::iterator Json::end() {
        if (auto obj{ EnsureMut<Object>() })
            return iterator{ (**obj)->end() };
        if (auto arr{ EnsureMut<Array>() })
            return iterator{ (*arr)->end(), (*arr)->size() };

        return {};
    }

    Json::const_iterator Json::begin() const {
        if (auto obj{ EnsureRef<Object>() })
            return const_iterator{ (**obj)->begin() };
        if (auto arr{ EnsureRef<Array>() })
            return const_iterator{ (*arr)->begin(), 0 };

        return {};
    }

    Json::const_iterator Json::end() const {
        if (auto obj{ EnsureRef<Object>() })
            return const_iterator{ (**obj)->end() };
        if (auto arr{ EnsureRef<Array>() })
            return const_iterator{ (*arr)->end(), (*arr)->size() };

        return {};
    }

    Json::const_iterator Json::cbegin() const {
        return begin();
    }

    Json::const_iterator Json::cend() const {
        return end();
    }


    bool Json::Empty() const {
        return std::visit(Hermes::Utils::Overloaded{
            [](const Array& arr) -> bool { return arr.empty(); },
            [](const Object& obj) -> bool { return obj->Empty(); },
            [](const auto&) -> bool { return true; }
        }, m_value);
    }

    bool Json::empty() const {
        return Empty();
    }

    size_t Json::Size() const {
        return std::visit(Hermes::Utils::Overloaded{
            [](const Array& arr) -> size_t { return arr.size(); },
            [](const Object& obj) -> size_t {
                // Prefers the object's own size when it has one, falls back to a ranges count.
                if constexpr (requires { obj->Size(); })
                    return obj->Size();
                else
                    return static_cast<size_t>(std::ranges::distance(*obj));
            },
            [](const auto&) -> size_t { return size_t{ 0 }; }
        }, m_value);
    }

    size_t Json::size() const {
        return Size();
    }


    JsonItemsView<false> Json::Items() {
        return JsonItemsView<false>{ this };
    }

    JsonItemsView<true> Json::Items() const {
        return JsonItemsView<true>{ this };
    }

#pragma endregion



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




static_assert(std::ranges::input_range<Json>);
static_assert(std::ranges::forward_range<Json>);
static_assert(std::ranges::bidirectional_range<Json>);
static_assert(std::ranges::common_range<Json>);
static_assert(std::ranges::sized_range<Json>);

static_assert(std::ranges::input_range<const Json>);
static_assert(std::ranges::forward_range<const Json>);
static_assert(std::ranges::bidirectional_range<const Json>);
static_assert(std::ranges::common_range<const Json>);
static_assert(std::ranges::sized_range<const Json>);

static_assert(std::ranges::view<JsonItemsView<true>>);
static_assert(std::ranges::input_range<JsonItemsView<true>>);
static_assert(std::ranges::common_range<JsonItemsView<true>>);
static_assert(std::ranges::sized_range<JsonItemsView<true>>);
static_assert(std::ranges::borrowed_range<JsonItemsView<true>>);
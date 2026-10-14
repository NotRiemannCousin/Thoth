#pragma once

namespace Thoth::NJson {

    constexpr Key::Key(const int i) : variant{ i } {}

    template<class T>
        requires std::constructible_from<JsonObjKey, T> &&
                 (!std::integral<std::remove_cvref_t<T>>) &&
                 (!std::same_as<std::remove_cvref_t<T>, Key>) &&
                 (!std::same_as<std::remove_cvref_t<T>, KeyView>)
    constexpr Key::Key(T&& str) : variant{ JsonObjKey{ std::forward<T>(str) } } {}

    constexpr Key::Key(const KeyView& view) {
        std::visit([this](const auto& v) {
            using T = std::remove_cvref_t<decltype(v)>;
            if constexpr (std::same_as<T, int>) {
                this->emplace<int>(v);
            } else {
                this->emplace<JsonObjKey>(JsonObjKey{ v });
            }
        }, static_cast<const std::variant<int, JsonObjKeyRef>&>(view));
    }

    inline bool Key::IsInt() const noexcept { return std::holds_alternative<int>(*this); }
    inline bool Key::IsString() const noexcept { return std::holds_alternative<JsonObjKey>(*this); }

    inline bool Key::operator==(const Key& other) const noexcept {
        return static_cast<const std::variant<int, JsonObjKey>&>(*this) == 
               static_cast<const std::variant<int, JsonObjKey>&>(other);
    }
    
    inline bool Key::operator==(const KeyView& other) const noexcept {
        return KeyView{ *this } == other;
    }

    constexpr KeyView::KeyView(const int i) : variant{ i } {}

    template<class T>
        requires std::constructible_from<JsonObjKeyRef, T> &&
                 (!std::integral<std::remove_cvref_t<T>>) &&
                 (!std::same_as<std::remove_cvref_t<T>, KeyView>) &&
                 (!std::same_as<std::remove_cvref_t<T>, Key>)
    constexpr KeyView::KeyView(T&& str) : variant{ JsonObjKeyRef{ std::forward<T>(str) } } {}

    constexpr KeyView::KeyView(const Key& key) {
        std::visit([this](const auto& v) {
            using T = std::remove_cvref_t<decltype(v)>;
            if constexpr (std::same_as<T, int>) {
                this->emplace<int>(v);
            } else {
                this->emplace<JsonObjKeyRef>(JsonObjKeyRef{ v });
            }
        }, static_cast<const std::variant<int, JsonObjKey>&>(key));
    }

    inline bool KeyView::IsInt() const noexcept { return std::holds_alternative<int>(*this); }
    inline bool KeyView::IsString() const noexcept { return std::holds_alternative<JsonObjKeyRef>(*this); }

    inline bool KeyView::operator==(const KeyView& other) const noexcept {
        return static_cast<const std::variant<int, JsonObjKeyRef>&>(*this) == 
               static_cast<const std::variant<int, JsonObjKeyRef>&>(other);
    }
    
    inline bool KeyView::operator==(const Key& other) const noexcept {
        return *this == KeyView{ other };
    }
}

template<>
struct std::formatter<Thoth::NJson::Key> {
    constexpr auto parse(auto& ctx) { return ctx.begin(); }

    auto format(const Thoth::NJson::Key& k, auto& ctx) const {
        return std::visit([&](const auto& v) {
            return std::format_to(ctx.out(), "{}", v);
        }, static_cast<const std::variant<int, Thoth::NJson::JsonObjKey>&>(k));
    }
};

template<>
struct std::formatter<Thoth::NJson::KeyView> {
    constexpr auto parse(auto& ctx) { return ctx.begin(); }

    auto format(const Thoth::NJson::KeyView& k, auto& ctx) const {
        return std::visit([&](const auto& v) {
            return std::format_to(ctx.out(), "{}", v);
        }, static_cast<const std::variant<int, Thoth::NJson::JsonObjKeyRef>&>(k));
    }
};
#pragma once
#include <variant>
#include <string>
#include <string_view>
#include <concepts>
#include <format>

namespace Thoth::NJson {

    using JsonObjKey    = std::string;
    using JsonObjKeyRef = std::string_view;

    struct KeyView;

    //! @brief A variant representing a JSON object key (string) or an array index (int).
    //! @details Inherits from std::variant to seamlessly hold an owned string or an integer index. This is typically
    //! used to build persistent JSON paths or store keys internally without losing type strictness.
    struct Key : std::variant<int, JsonObjKey> {
        using variant::variant;

        //! @brief Default constructor. Initializes with index 0.
        constexpr Key() = default;

        //! @brief Constructs a key from an integer index.
        //! @param i The array index.
        constexpr Key(int i);

        //! @brief Constructs a key from any string-like object.
        //! @tparam T Type convertible to JsonObjKey (std::string).
        //! @param str The string-like key.
        template<class T>
            requires std::constructible_from<JsonObjKey, T> &&
                     (!std::integral<std::remove_cvref_t<T>>) &&
                     (!std::same_as<std::remove_cvref_t<T>, Key>) &&
                     (!std::same_as<std::remove_cvref_t<T>, KeyView>)
        constexpr Key(T&& str);

        //! @brief Constructs an owned Key by explicitly copying data from a non-owning KeyView.
        //! @param view The KeyView to copy from.
        constexpr Key(const KeyView& view);

        //! @return True if the key represents an array index.
        [[nodiscard]] bool IsInt() const noexcept;

        //! @return True if the key represents an object key (string).
        [[nodiscard]] bool IsString() const noexcept;

        //! @brief Checks equality against another Key.
        bool operator==(const Key& other) const noexcept;

        //! @brief Checks equality against a KeyView.
        bool operator==(const KeyView& other) const noexcept;
    };

    //! @brief A non-owning variant representing a JSON object key (string_view) or an array index (int).
    //! @details Used for fast, allocation-free lookups (e.g., Get, Find) in JSON trees.
    struct KeyView : std::variant<int, JsonObjKeyRef> {
        using variant::variant;

        //! @brief Default constructor. Initializes with index 0.
        constexpr KeyView() = default;

        //! @brief Constructs a KeyView from an integer index.
        //! @param i The array index.
        constexpr KeyView(int i);

        //! @brief Constructs a KeyView from any string-like object.
        //! @tparam T Type convertible to JsonObjKeyRef (std::string_view).
        //! @param str The string-like key.
        template<class T>
            requires std::constructible_from<JsonObjKeyRef, T> &&
                     (!std::integral<std::remove_cvref_t<T>>) &&
                     (!std::same_as<std::remove_cvref_t<T>, KeyView>) &&
                     (!std::same_as<std::remove_cvref_t<T>, Key>)
        constexpr KeyView(T&& str);

        //! @brief Constructs a KeyView viewing an existing owned Key.
        //! @param key The owned Key to view.
        constexpr KeyView(const Key& key);

        //! @return True if the KeyView represents an array index.
        [[nodiscard]] bool IsInt() const noexcept;

        //! @return True if the KeyView represents an object key (string).
        [[nodiscard]] bool IsString() const noexcept;

        //! @brief Checks equality against another KeyView.
        bool operator==(const KeyView& other) const noexcept;

        //! @brief Checks equality against an owned Key.
        bool operator==(const Key& other) const noexcept;
    };

    using Keys     = std::span<const Key>;
    using KeysView = std::span<const KeyView>;
}

#include <Thoth/NJson/Keys.tpp>
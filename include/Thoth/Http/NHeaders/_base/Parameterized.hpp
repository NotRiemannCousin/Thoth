#pragma once
#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <functional>
#include <type_traits>

#include <Thoth/Utils/Scanner.hpp>

namespace Thoth::Http::NHeaders {
    //! @brief Associates a serializable header value with arbitrary named parameters.
    //!
    //! The wire form is a base value followed by zero or more 'key=value` parameters separated by ';'. Parameter keys
    //! are matched case-insensitively, while values are stored as strings. The template is intended for open-ended
    //! parameter sets such as media types, links and content dispositions.
    //!
    //! Headers with fixed, semantically rich attributes use their own dedicated types instead (e.g. `Cookie` and
    //! `Challenge`_.
    //!
    //! @tparam T Serializable base value carried by the parameterized header.
    //!
    //! @par Example
    //! @code{.cpp}
    //! using Preference = Parameterized<std::string>;
    //! Preference wait{ "wait" };
    //! auto withSeconds{ wait.WithParam("seconds", "10") };
    //! @endcode
    template<Utils::Serializable T>
    struct Parameterized {
        //! Parsed base value before the parameter list.
        T value{};
        //! Parameter key/value pairs in their parsed order.
        std::vector<std::pair<std::string, std::string>> params{};

        bool operator==(const Parameterized&) const;
        bool operator==(const T& t) const;

        //! @brief Looks up a parameter by key.
        //! @param key Parameter name, compared case-insensitively.
        //! @return The parameter's value, or `std::nullopt` if `key` is not present.
        [[nodiscard]] std::optional<std::string_view> Param(std::string_view key) const;

        //! @brief Returns a copy with `key` set to `val`, replacing any existing parameter with that key.
        //! @param key Parameter name.
        //! @param val Parameter value.
        [[nodiscard]] Parameterized WithParam(std::string key, std::string val) const&;
        //! @copydoc WithParam
        [[nodiscard]] Parameterized WithParam(std::string key, std::string val) &&;

        //! @brief Returns a copy with any parameter named `key` removed. No-op if `key` is not present.
        //! @param key Parameter name, compared case-insensitively.
        [[nodiscard]] Parameterized WithoutParam(std::string_view key) const&;
        //! @copydoc WithoutParam
        [[nodiscard]] Parameterized WithoutParam(std::string_view key) &&;

        //! @brief Applies `fn` to the wrapped value, keeping the same parameters.
        //! @param fn Transformation applied to `value`.
        //! @return A parameterized wrapper around the transformed value.
        //! @note Meant for use with `std::optional::transform`/`std::expected::transform`, see
        //! @ref H_Transform for a curried form ready to drop into a `.transform(...)` chain.
        template<class Fn>
        auto Transform(Fn&& fn) const -> Parameterized<std::invoke_result_t<Fn, const T&>>
            requires Utils::Serializable<std::invoke_result_t<Fn, const T&>>;

        //! @hof{Transform}
        template<class Fn>
        static auto H_Transform(Fn fn);
    };
}

#include <Thoth/Http/NHeaders/_base/Parameterized.tpp>


namespace Thoth::Http::NHeaders {
    //! @brief Parameterized version of `std::string`.
    using FreeParameterizedHeader = Parameterized<std::string>;

    static_assert(Thoth::Utils::Serializable<FreeParameterizedHeader>);
}

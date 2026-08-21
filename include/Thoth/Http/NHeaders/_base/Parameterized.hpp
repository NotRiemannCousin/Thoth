#pragma once
#include <string>
#include <vector>
#include <utility>
#include <optional>
#include <functional>
#include <type_traits>

#include <Thoth/Utils/Scanner.hpp>

namespace Thoth::Http::NHeaders {
    //! @brief Wraps a header value with a generic, unbounded `;key=value` parameter list
    //! (RFC 9110 §5.6.6 parameters grammar).
    //!
    //! @note Only for genuinely open-ended parameter sets (arbitrary keys, arbitrary count) attached to a base value.
    //! Headers whose params have fixed, known semantics keep their own dedicated struct (@ref Cookie, @ref Challenge)
    //! - Parameterized would just hide the shape. Headers with no base value at all (e.g. Forwarded) don't fit here
    //! either.
    template<Utils::Serializable T>
    struct Parameterized {
        T value{};
        std::vector<std::pair<std::string, std::string>> params{};

        bool operator==(const Parameterized&) const;
        bool operator==(const T& t) const;

        //! @brief Looks up a parameter by key (case-insensitive, per RFC 9110 §5.6.6 token comparison).
        //! @return The parameter's value, or nullopt if `key` isn't present.
        [[nodiscard]] std::optional<std::string_view> Param(std::string_view key) const;

        //! @brief Returns a copy with `key` set to `val`, replacing any existing parameter with that key.
        [[nodiscard]] Parameterized WithParam(std::string key, std::string val) const&;
        //! @copydoc WithParam
        [[nodiscard]] Parameterized WithParam(std::string key, std::string val) &&;

        //! @brief Returns a copy with any parameter named `key` removed. No-op if `key` isn't present.
        [[nodiscard]] Parameterized WithoutParam(std::string_view key) const&;
        //! @copydoc WithoutParam
        [[nodiscard]] Parameterized WithoutParam(std::string_view key) &&;

        //! @brief Applies `fn` to the wrapped value, keeping the same parameters.
        //! @note Meant for use with `std::optional::transform`/`std::expected::transform`; see
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
    using FreeParameterizedHeader = Parameterized<std::string>;

    static_assert(Thoth::Utils::Serializable<FreeParameterizedHeader>);
}
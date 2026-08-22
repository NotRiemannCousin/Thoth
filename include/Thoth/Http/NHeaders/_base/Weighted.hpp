#pragma once
#include <functional>
#include <type_traits>

#include <Thoth/Utils/Scanner.hpp>

namespace Thoth::Http::NHeaders {
    //! @brief Associates a header value with an RFC 9110 §12.4.2 quality weight.
    //!
    //! The `q` member is in the inclusive range `0.000` to `1.000` and defaults to `1.0` when the wire value does not
    //! contain a `q` parameter. This type is used for comma-separated preference headers such as `Accept-Encoding` and
    //! `Accept-Language`.
    //!
    //! `Weighted<T>` stores the parsed value in `value` and the preference in `q`. It only extracts the `;q=` segment.
    //!
    //! @tparam T Serializable value carried by the weighted header element.
    //!
    //! @par Example
    //! @code{.cpp}
    //! using Language = Weighted<std::string>;
    //! Language preferred{ "pt-BR", 1.0 };
    //! auto fallback{ preferred.WithWeight(0.8) };
    //! @endcode
    template<Utils::Serializable T>
    struct Weighted {
        //! Parsed header value.
        T value{};
        //! Preference weight, normally serialized with three decimal places.
        double q{ 1.0 };

        bool operator==(const Weighted&) const;
        bool operator==(const T&) const;

        //! @brief Returns a copy with the weight replaced by `newQ`, keeping the same value.
        //! @param newQ New preference weight.
        [[nodiscard]] Weighted WithWeight(double newQ) const;

        //! @brief Applies `fn` to the wrapped value, keeping the same weight.
        //! @param fn Transformation applied to `value`.
        //! @return A weighted wrapper around the transformed value.
        //! @note Meant for use with `std::optional::transform`/`std::expected::transform`; see
        //! @ref H_Transform for a curried form ready to drop into a `.transform(...)` chain.
        template<class Fn>
        auto Transform(Fn&& fn) const -> Weighted<std::invoke_result_t<Fn, const T&>>
            requires Utils::Serializable<std::invoke_result_t<Fn, const T&>>;

        //! @brief Curried form of `Transform` for use in a monadic pipeline.
        template<class Fn>
        static auto H_Transform(Fn fn);
    };
}

#include <Thoth/Http/NHeaders/_base/Weighted.tpp>

namespace Thoth::Http::NHeaders {
    //! @brief Weighted version of `std::string`.
    using FreeWeightedHeader = Weighted<std::string>;

    static_assert(Thoth::Utils::Serializable<FreeWeightedHeader>);
}

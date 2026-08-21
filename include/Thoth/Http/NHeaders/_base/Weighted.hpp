#pragma once
#include <functional>
#include <type_traits>

#include <Thoth/Utils/Scanner.hpp>

namespace Thoth::Http::NHeaders {
    //! @brief Wraps a header value with an RFC 9110 §12.4.2 `q` weight (0.000-1.000, default 1.0).
    //!
    //! @note Only extracts the `;q=` segment from the raw value. Any other parameter present (media-type params before
    //! `q` on Accept, accept-ext after it) is left untouched for `T`'s own Scanner - Weighted doesn't otherwise
    //! interpret the grammar of `T`.
    template<Utils::Serializable T>
    struct Weighted {
        T value{};
        double q{ 1.0 };

        bool operator==(const Weighted&) const;
        bool operator==(const T&) const;

        //! @brief Returns a copy with the weight replaced by `newQ`, keeping the same value.
        [[nodiscard]] Weighted WithWeight(double newQ) const;

        //! @brief Applies `fn` to the wrapped value, keeping the same weight.
        //! @note Meant for use with `std::optional::transform`/`std::expected::transform`; see
        //! @ref H_Transform for a curried form ready to drop into a `.transform(...)` chain.
        template<class Fn>
        auto Transform(Fn&& fn) const -> Weighted<std::invoke_result_t<Fn, const T&>>
            requires Utils::Serializable<std::invoke_result_t<Fn, const T&>>;

        //! @hof{Transform}
        template<class Fn>
        static auto H_Transform(Fn fn);
    };
}

#include <Thoth/Http/NHeaders/_base/Weighted.tpp>

namespace Thoth::Http::NHeaders {
    using FreeWeightedHeader = Weighted<std::string>;

    static_assert(Thoth::Utils::Serializable<FreeWeightedHeader>);
}
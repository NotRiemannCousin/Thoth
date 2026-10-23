#pragma once

#include <cstddef>
#include <type_traits>
#include <variant>

namespace Thoth::Utils {

    template<class T, class Variant, template<class, class> class Relation, std::size_t Index = 0>
    consteval std::size_t FindMatchIndexImpl() {
        constexpr std::size_t k_size{ std::variant_size_v<Variant> };
        static_assert(Index != k_size, "No type in the variant satisfies the specified relation.");

        if constexpr (Relation<T, std::variant_alternative_t<Index, Variant>>::value)
            return Index;
        else
            return FindMatchIndexImpl<T, Variant, Relation, Index + 1>();
    }

    //! @brief Returns the index of the first alternative of `Variant` that satisfies the relation.
    template<class T, class Variant, template<class, class> class Relation>
    consteval std::size_t FindMatchIndex() {
        return FindMatchIndexImpl<T, Variant, Relation>();
    }


    //! @brief The first alternative of `Variant` that can be coverted from `T` (e.g. `const char*` converts to
    //! `std::string` in `std::variant<int, std::string>`).
    template<class T, class Variant>
    using FirstConvertibleVariant = std::variant_alternative_t<FindMatchIndex<T, Variant, std::is_convertible>(), Variant>;

    //! @brief The first alternative of `Variant` that is exactly `T`.
    template<class T, class Variant>
    using FirstEqualVariant = std::variant_alternative_t<FindMatchIndex<T, Variant, std::is_same>(), Variant>;
}

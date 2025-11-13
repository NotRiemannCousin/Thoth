#pragma once

#include <variant>

namespace Thoth::Utils {
    template<class T, class Variant, template<class, class> class Relation, std::size_t Index = 0>
    consteval size_t FindMatchIndexImpl() {
        constexpr size_t size{ std::variant_size_v<Variant> };
        static_assert(Index != size, "No type in the variant satisfies the specified relation.");

        if constexpr (Relation<T, std::variant_alternative_t<Index, Variant>>::value)
            return Index;


        return FindMatchIndexImpl<T, Variant, Relation, Index + 1>();
    }

    template<class T, class Variant, template<class, class> class Relation>
    consteval std::size_t FindMatchIndex() {
        return FindMatchIndexImpl<T, Variant, Relation>();
    }


    template<class T, class Variant>
    using FirstConvertibleVariant = std::variant_alternative_t<FindMatchIndex<T, Variant, std::is_convertible>(), Variant>;

    template<class T, class Variant>
    using FirstEqualVariant = std::variant_alternative_t<FindMatchIndex<T, Variant, std::is_same>(), Variant>;
}
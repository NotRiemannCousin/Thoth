#pragma once

#include <variant>

namespace Thoth::Utils {
    template<class T, class Variant, std::size_t... I>
    consteval std::size_t FindConvertibleIndexImpl(std::index_sequence<I...>) {
        constexpr std::size_t size{ std::variant_size_v<Variant> };
        std::size_t result{ size };

        ((result == size && std::convertible_to<T, std::variant_alternative_t<I, Variant>> ? result = I : 0), ...);

        return result;
    }

    template<class T, class Variant>
    consteval std::size_t FindConvertibleIndex() {
        return FindConvertibleIndexImpl<T, Variant>(
            std::make_index_sequence<std::variant_size_v<Variant>>{});
    }


    template<class T, class Variant>
    using FirstConvertibleVariant = std::variant_alternative_t<FindConvertibleIndex<T, Variant>(), Variant>;
}
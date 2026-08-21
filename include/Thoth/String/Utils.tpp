#pragma once
#include <algorithm>


namespace Thoth::String {
    template<std::ranges::range R>
    std::string_view ForceView(R &&str) {
        auto l{ std::ranges::begin(str) };
        auto r{ std::ranges::end(str) };

            return { std::ranges::data(str), std::ranges::size(str) };
    }

    constexpr auto H_ForceView() {
        return [](auto&& r) { return ForceView(r); };
    }

    constexpr std::bitset<256> MakeBitset(const std::initializer_list<std::string_view> strs) {
        std::bitset<256> bits{};

        for (auto sv : strs)
            for (char c : sv)
                bits.set(static_cast<unsigned char>(c));

        return bits;
    }

}

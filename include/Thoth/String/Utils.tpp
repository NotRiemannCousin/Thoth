#pragma once
#include <algorithm>


namespace Thoth::String {
    constexpr std::bitset<256> MakeBitset(const std::initializer_list<std::string_view> strs) {
        std::bitset<256> bits{};

        for (auto sv : strs)
            for (char c : sv)
                bits.set(static_cast<unsigned char>(c));

        return bits;
    }

}

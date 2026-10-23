#pragma once
#include <algorithm>


namespace Thoth::String {
    template<std::ranges::range R>
    std::string_view ForceView(R &&str) {
        auto l{ std::ranges::begin(str) };
        auto r{ std::ranges::end(str) };

            return { std::ranges::data(str), std::ranges::size(str) };
    }

    template<class R1, class R2>
    bool TokenEquals(const R1& a, const R2& b) {
        namespace rg = std::ranges;
        return rg::size(a) == rg::size(b) && rg::equal(a, b, CaseInsensitiveCompare);
    }

    constexpr auto H_TokenEquals() {
        return []<class R1, class R2>(R1&& a, R2&& b) {
            return TokenEquals(a, b);
        };
    }

    template<class R1>
    constexpr auto H_TokenEquals(R1&& a) {
        return [a = std::forward<R1>(a)]<class R2>(R2&& b) {
            return TokenEquals(a, b);
        };
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

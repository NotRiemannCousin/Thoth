#pragma once


namespace Thoth::Utils {
    constexpr void HashCombine(size_t& seed, const size_t v) {
        seed ^= v + 0x9e3779b97f4a7c15ULL + (seed<<6) + (seed>>2);
    }
}
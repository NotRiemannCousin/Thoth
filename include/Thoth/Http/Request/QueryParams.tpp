#pragma once
#include <ranges>

namespace std {
    template<>
    struct hash<Thoth::Http::QueryParams> {
        size_t operator()(const Thoth::Http::QueryParams& params) const noexcept {
            using Thoth::Utils::HashCombine;
            size_t seed = 1469598103934665603ULL;

            for (const auto& [key, vals] : params) {
                HashCombine(seed, std::hash<Thoth::Http::QueryParams::QueryKey>{}(key));

                for (const auto& val : vals)
                    HashCombine(seed, std::hash<Thoth::Http::QueryParams::QueryValue>{}(val));
            }
            HashCombine(seed, std::hash<size_t>{}(params.Size()));
            return seed;
        }
    };

    template<>
    struct formatter<Thoth::Http::QueryParams>{

        static constexpr auto parse(auto &ctx) { return ctx.begin(); }

        auto format(const Thoth::Http::QueryParams &query, std::format_context &ctx) const {
#ifdef __cpp_lib_ranges_concat
            static const auto groupParams = [](const auto& p){
                return std::views::concat(p.first, std::views::single('='), p.second | std::views::join_with(','));
            };
#else
            static const auto groupParams = [](const auto& p){
                return p.first + '=' + (p.second | std::views::join_with(',') | std::ranges::to<string>());
            };
#endif

            return std::ranges::copy(query._elements | std::views::transform(groupParams) | std::views::join_with('&'), ctx.out()).out;
        }
    };
}

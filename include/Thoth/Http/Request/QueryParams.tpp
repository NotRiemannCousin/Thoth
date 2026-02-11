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
            using Pair = std::pair<string_view, string_view>;

            static constexpr auto singleParam = [] (const Pair p) {
                return std::format("{}={}", p.first, p.second);
#ifdef __cpp_lib_ranges_concat
                return std::views::concat(p.first, std::views::single('='), p.second | std::views::join_with(','));
#else
            };
            static constexpr auto getEveryPair = [](const Thoth::Http::QueryParams::MapType::value_type& p) {
                return p.second | views::transform([&](string_view val) { return Pair{ p.first, val }; });
            };
#endif

            return std::ranges::copy(
                query._elements
                        | views::transform(getEveryPair)
                        | views::join
                        | std::views::transform(singleParam)
                        | views::join_with('&'),
                ctx.out()).out;
        }
    };
}

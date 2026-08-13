#pragma once
#include <ranges>

namespace std {
    template<>
    struct hash<Thoth::Http::QueryParams> {
        size_t operator()(const Thoth::Http::QueryParams& params) const noexcept {
            using Hermes::Utils::HashCombine;
            size_t seed{ 1469598103934665603ULL };

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

        template<class FormatContext>
        auto format(const Thoth::Http::QueryParams &query, FormatContext& ctx) const {
            using Pair = std::pair<std::string_view, std::string_view>;
            namespace vs = std::views;

            static constexpr auto getEveryPair{ [](const Thoth::Http::QueryParams::MapType::value_type& p) {
                return p.second | vs::transform([&](std::string_view val) { return Pair{ p.first, val }; });
            } };

            static constexpr auto singleParam{ [] (const Pair p) {
#ifdef __cpp_lib_ranges_concat
                return vs::concat(p.first, vs::single('='), p.second);
#else
                return std::format("{}={}", p.first, p.second);
#endif
            } };

            return std::ranges::copy(
                query.m_elements
                        | vs::transform(getEveryPair)
                        | vs::join
                        | vs::transform(singleParam)
                        | vs::join_with('&'),
                ctx.out()).out;
        }
    };
}

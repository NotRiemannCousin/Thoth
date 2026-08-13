#pragma once


template<>
struct std::formatter<Thoth::NJson::Number> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

    //! Outputs the value in its natural format.
    //! @note @c double values with no fractional part are written with a trailing
    //! @c ".0" (e.g. @c 1.0 → @c "1.0") to preserve round-trip fidelity through JSON.
    template<class FormatContext>
    auto format(const Thoth::NJson::Number& n, FormatContext& ctx) const{
        auto out{ ctx.out() };
        std::visit([&]<class T>(const T val) {
            if constexpr (std::same_as<T, double>) {
                const auto s{ std::format("{}", val) };
                out = std::ranges::copy(s, out).out;
                // Doubles with no fractional part (e.g. "1") must be written as "1.0"
                // so they round-trip as double instead of being re-parsed as uint64_t.
                if (s.find('.') == std::string::npos && s.find('e') == std::string::npos)
                    out = std::ranges::copy(std::string_view{ ".0" }, out).out;
            } else {
                out = std::format_to(out, "{}", val);
            }
        }, static_cast<const std::variant<int64_t, uint64_t, double>&>(n));
        return out;
    }
};

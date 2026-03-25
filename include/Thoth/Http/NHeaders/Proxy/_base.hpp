#pragma once
#include <string_view>
#include <optional>
#include <variant>
#include <iomanip>
#include <format>
#include <chrono>
#include <string>

#include <Thoth/String/Utils.hpp>
#include <Thoth/Utils/Monostate.hpp>

namespace Thoth::Http::NHeaders {
    template<class T>
    struct Scanner {
        Scanner()                    = delete;
        Scanner(Scanner&)            = delete;
        Scanner& operator=(Scanner&) = delete;
    };

    template<class T>
    std::optional<T> Scan(std::string_view input, std::string_view args = "") {
        Scanner<T> scanner{};

        if (!scanner.Parse(args))
            return std::nullopt;

        return scanner.Scan(input);
    }
    // TODO: Implement Scan in a better way.


    template<class T>
    concept Scannable = requires (Scanner<std::remove_cvref_t<T>> scanner, std::string_view str) {
        { scanner.Parse(str) } -> std::same_as<bool>;
        { scanner.Scan(str)  } -> std::same_as<std::optional<std::remove_cvref_t<T>>>;
    };

    template<class T>
    concept Serializable = Scannable<T> && std::formattable<T, char>;


    enum class HeaderErrorEnum {
        NotFound,
        InvalidFormat,
        EmptyValue
    };
    struct InvalidHeaderFormat {};






    template<std::integral T>
    struct Scanner<T> {
    private:
        int base{ 10 };
    public:
        bool Parse(const std::string_view str) {
            if (str.empty()) return true;
            if (str == "b") { base =  2; return true; }
            if (str == "x") { base = 16; return true; }

            const auto [_, ec]{ std::from_chars(str.data(), str.data() + str.size(), base) };


            return ec == std::errc() && base >= 1 && base <= 36;
        }
        std::optional<T> Scan(const std::string_view str) {
            T val;
            const auto [_, ec]{ std::from_chars(str.data(), str.data() + str.size(), val, base) };

            if (ec == std::errc())
                return val;

            return std::nullopt;
        }
    };

    static_assert(Serializable<int>);

    template<>
    struct Scanner<std::string> {
        static bool Parse(const std::string_view str) {
            return str.empty();
        }

        static std::optional<std::string> Scan(std::string_view input) {
            return String::TrimmedStr(input);
        }
    };

    static_assert(Serializable<std::string>);

    template<>
    struct Scanner<std::chrono::utc_clock::time_point> {
        std::string pattern{ "%a, %d %b %Y %H:%M:%S GMT" };

        static bool Parse(const std::string_view str) {
            if (str.empty()) return true;

            std::tm tm{};
            std::istringstream ss{ std::string{ str } };
            ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S GMT");
            return !ss.fail();
        }

        static std::optional<std::chrono::utc_clock::time_point> Scan(std::string_view input) {
            using TimePoint = std::chrono::utc_clock::time_point;
            static constexpr char outputFormat[]{ "%Y-%m-%d %H:%M:%S %z" };

            if (input.empty()) return std::nullopt;

            std::istringstream ss{ std::string{ input } };


            if (TimePoint clock; std::chrono::from_stream(ss, outputFormat, clock))
                return clock;

            return std::nullopt;
        }
    };

    static_assert(Serializable<std::chrono::utc_clock::time_point>);

    template<class Ped, class Ratio>
    struct Scanner<std::chrono::duration<Ped, Ratio>> {
        using Duration = std::chrono::duration<Ped, Ratio>;

        static bool Parse(const std::string_view str) {
            return str.empty();
        }

        static std::optional<Duration> Scan(std::string_view input) {
            if (input.empty()) return std::nullopt;

            if (const auto timeAsNumber{ NHeaders::Scan<Ped>(input) }; timeAsNumber)
                return { *timeAsNumber };

            return std::nullopt;
        }
    };



    // std::format("{}", dur) will print with a type suffix (like 350min or 180s). It must be "{:Q}".
    // TODO: creates a pattern to output too.
    static_assert(Serializable<std::chrono::seconds>);


    template<>
    struct Scanner<std::monostate> {
        static bool Parse(const std::string_view str) {
            return str.empty();
        }

        static std::optional<std::monostate> Scan(std::string_view input) {
            if (String::TrimmedStr(input) == "null")
                return  std::monostate{};
            return std::nullopt;
        }
    };

    static_assert(Serializable<std::monostate>);
}

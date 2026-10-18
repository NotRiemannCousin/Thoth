#pragma once
/// @file BenchJson_Shared.hpp
/// @brief Common infrastructure shared by all JSON benchmark translation units.
///        Each TU includes this header and defines its own library-specific benchmarks.

#include <array>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <utility>

namespace fs = std::filesystem;

// ======================================================================
//  File loader
// ======================================================================

inline static std::string ReadFile(const char* rel) {
    const fs::path base{ BENCH_DATA_DIR };
    const fs::path full{ base / rel };
    std::ifstream f{ full, std::ios::binary };
    if (!f)
        throw std::runtime_error{ std::string("Cannot open: ") + full.string() };
    return { std::istreambuf_iterator<char>{f}, {} };
}

// ======================================================================
//  Lazy-loaded, process-wide datasets
// ======================================================================

struct Dataset {
    std::string small;
    std::string medium;
    std::string large;
    std::string nested;
    std::string numbers;
    std::string array;
    std::string strings;
    std::string rawStrings;
    std::string twitter;

    static const Dataset& Get() {
        static const Dataset d = [] {
            Dataset x{
                .small      = ReadFile("small.json"),
                .medium     = ReadFile("medium.json"),
                .large      = ReadFile("large.json"),
                .nested     = ReadFile("nested.json"),
                .numbers    = ReadFile("numbers.json"),
                .array      = ReadFile("array.json"),
                .strings    = ReadFile("strings.json"),
                .rawStrings = ReadFile("raw_strings.json"),
                .twitter    = ReadFile("twitter.json"),
            };
            return x;
        }();
        return d;
    }
};

// ======================================================================
//  Dataset selector
// ======================================================================

enum class DS { Small, Medium, Large, Nested, Numbers, Array, Strings, RawStrings, Twitter };

inline static const std::string& Pick(DS ds) {
    const auto& d{ Dataset::Get() };
    switch (ds) {
        case DS::Small:      return d.small;
        case DS::Medium:     return d.medium;
        case DS::Large:      return d.large;
        case DS::Nested:     return d.nested;
        case DS::Numbers:    return d.numbers;
        case DS::Array:      return d.array;
        case DS::Strings:    return d.strings;
        case DS::RawStrings: return d.rawStrings;
        case DS::Twitter:    return d.twitter;
    }
    std::unreachable();
}

inline static constexpr const char* DSName(DS ds) {
    switch (ds) {
        case DS::Small:      return "small";
        case DS::Medium:     return "medium";
        case DS::Large:      return "large";
        case DS::Nested:     return "nested";
        case DS::Numbers:    return "numbers";
        case DS::Array:      return "array";
        case DS::Strings:    return "strings";
        case DS::RawStrings: return "raw_strings";
        case DS::Twitter:    return "twitter";
    }
    std::unreachable();
}
// ======================================================================
//  Canonical benchmark names
// ======================================================================
// Every registered benchmark follows: Suite/Library/Variant/Test.

#define BENCH_NAME_IMPL(suite, library, variant, test) \
    #suite "/" #library "/" #variant "/" #test
#define BENCH_NAME(suite, library, variant, test) \
    BENCH_NAME_IMPL(suite, library, variant, test)

#define BENCH_PARSE_DS(lib, DS_ENUM) \
    BENCHMARK_TEMPLATE(BM_##lib##_Parse, DS::DS_ENUM)->Name(BENCH_NAME(Parse, lib, Base, DS_ENUM))

#define BENCH_PARSE_ALL(lib)         \
    BENCH_PARSE_DS(lib, Small);      \
    BENCH_PARSE_DS(lib, Medium);     \
    BENCH_PARSE_DS(lib, Large);      \
    BENCH_PARSE_DS(lib, Nested);     \
    BENCH_PARSE_DS(lib, Numbers);    \
    BENCH_PARSE_DS(lib, Array);      \
    BENCH_PARSE_DS(lib, Strings);    \
    BENCH_PARSE_DS(lib, RawStrings); \
    BENCH_PARSE_DS(lib, Twitter);


#define BENCH_STR_DS(lib, DS_ENUM) \
    BENCHMARK_TEMPLATE(BM_##lib##_Stringify, DS::DS_ENUM)->Name(BENCH_NAME(Stringify, lib, Base, DS_ENUM))
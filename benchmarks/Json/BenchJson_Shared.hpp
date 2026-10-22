#pragma once
/// @file BenchJson_Shared.hpp
/// @brief Common infrastructure shared by all JSON benchmark translation units.
///        Each TU includes this header and defines its own library-specific benchmarks.

#include <filesystem>
#include <functional>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <utility>

namespace fs = std::filesystem;

#define BENCH_DATASETS(X, ...) \
    X(Small,      small       __VA_OPT__(,) __VA_ARGS__) \
    X(Medium,     medium      __VA_OPT__(,) __VA_ARGS__) \
    X(Large,      large       __VA_OPT__(,) __VA_ARGS__) \
    X(Nested,     nested      __VA_OPT__(,) __VA_ARGS__) \
    X(Numbers,    numbers     __VA_OPT__(,) __VA_ARGS__) \
    X(Array,      array       __VA_OPT__(,) __VA_ARGS__) \
    X(Strings,    strings     __VA_OPT__(,) __VA_ARGS__) \
    X(RawStrings, raw_strings __VA_OPT__(,) __VA_ARGS__) \
    X(Twitter,    twitter     __VA_OPT__(,) __VA_ARGS__) \
    X(Youtube,    youtube     __VA_OPT__(,) __VA_ARGS__)

// ======================================================================
//  File loader
// ======================================================================

inline static std::string ReadFile(const char* rel) {

    const fs::path full{fs::path{ BENCH_DATA_DIR } / rel};

    std::ifstream file{ full, std::ios::binary };
    if (!file) {
        throw std::runtime_error{ "Cannot open: " + full.string() };
    }

    return {
        std::istreambuf_iterator<char>{ file },
        std::istreambuf_iterator<char>{}
    };
}

// ======================================================================
//  Lazy-loaded, process-wide datasets
// ======================================================================

struct Dataset {
#define DECLARE_FIELD(id, name) std::string name;

    BENCH_DATASETS(DECLARE_FIELD)

#undef DECLARE_FIELD

    static const Dataset& Get() {
        static const Dataset dataset = [] {
            return Dataset{
#define LOAD_DATASET(id, name) \
                .name = ReadFile(#name ".json"),

                BENCH_DATASETS(LOAD_DATASET)

#undef LOAD_DATASET
            };
        }();

        return dataset;
    }
};

// ======================================================================
//  Dataset selector
// ======================================================================

enum class DS {
#define DECLARE_ENUM(id, name) id,

    BENCH_DATASETS(DECLARE_ENUM)

#undef DECLARE_ENUM
};

inline static const std::string& Pick(DS ds) {
    const auto& dataset = Dataset::Get();

    switch (ds) {
#define PICK_DATASET(id, name) case DS::id: return dataset.name;

        BENCH_DATASETS(PICK_DATASET)

#undef PICK_DATASET
    }

    std::unreachable();
}

static constexpr const char* DSName(DS ds) {
    switch (ds) {
#define NAME_DATASET(id, name) case DS::id: return #name;

        BENCH_DATASETS(NAME_DATASET)

#undef NAME_DATASET
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

#define BENCH_PARSE_DATASET(id, name, lib) \
    BENCHMARK_TEMPLATE(BM_##lib##_Parse, DS::id)->Name(BENCH_NAME(Parse, lib, Base, id));

#define BENCH_PARSE_ALL(lib) \
    BENCH_DATASETS(BENCH_PARSE_DATASET, lib)


#define BENCH_STR_DATASET(id, name, lib) \
    BENCHMARK_TEMPLATE(BM_##lib##_Stringify, DS::id)->Name(BENCH_NAME(Stringify, lib, Base, id));

#define BENCH_STR_ALL(lib) \
    BENCH_DATASETS(BENCH_STR_DATASET, lib)
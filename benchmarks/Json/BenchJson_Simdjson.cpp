/// @file BenchJson_Simdjson.cpp
/// @brief JSON benchmarks for simdjson (on-demand + DOM variants).

#include <benchmark/benchmark.h>

#include <simdjson.h>

#include "BenchJson_Shared.hpp"

namespace sj = simdjson;

// ======================================================================
//  SIMDJSON — Parse (on-demand)
// ======================================================================

template<DS ds>
static void BM_Simdjson_Parse(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    sj::ondemand::parser parser;
    sj::padded_string padded{ src };

    for (auto _ : state) {
        sj::ondemand::document doc = parser.iterate(padded);
        auto val{ doc.get_value() };
        benchmark::DoNotOptimize(val);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(src.size()));
    state.SetLabel(DSName(ds));
}

// ======================================================================
//  SIMDJSON — Parse (DOM style)
// ======================================================================

template<DS ds>
static void BM_Simdjson_DOM_Parse(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    sj::dom::parser parser;
    sj::padded_string padded{ src };

    for (auto _ : state) {
        sj::dom::element doc = parser.parse(padded);
        benchmark::DoNotOptimize(doc);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(src.size()));
    state.SetLabel(std::string(DSName(ds)) + "/dom");
}

// ======================================================================
//  SIMDJSON — Key Access (DOM)
// ======================================================================

static void BM_Simdjson_KeyAccess_Medium(benchmark::State& state) {
    sj::dom::parser parser;
    sj::padded_string padded{ Dataset::Get().medium };
    sj::dom::element doc = parser.parse(padded);

    for (auto _ : state) {
        auto total{ doc["total"].get_int64() };
        auto page { doc["page"].get_int64()  };
        auto users{ doc["users"].get_array() };
        benchmark::DoNotOptimize(total);
        benchmark::DoNotOptimize(page);
        benchmark::DoNotOptimize(users);
    }
}

// ======================================================================
//  SIMDJSON — Array Iteration (DOM)
// ======================================================================

static void BM_Simdjson_ArrayIteration_Array(benchmark::State& state) {
    sj::dom::parser parser;
    sj::padded_string padded{ Dataset::Get().array };
    sj::dom::element doc = parser.parse(padded);

    for (auto _ : state) {
        std::size_t count{};
        for (sj::dom::element elem : doc.get_array()) {
            auto label{ elem["label"].get_string() };
            benchmark::DoNotOptimize(label);
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
}

static void BM_Simdjson_ArrayIteration_Large(benchmark::State& state) {
    sj::dom::parser parser;
    sj::padded_string padded{ Dataset::Get().large };
    sj::dom::element doc = parser.parse(padded);

    for (auto _ : state) {
        std::size_t count{};
        for (sj::dom::element elem : doc["data"].get_array()) {
            auto name{ elem["name"].get_string() };
            benchmark::DoNotOptimize(name);
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
}

// ======================================================================
//  SIMDJSON — Path Traversal (DOM)
// ======================================================================

static void BM_Simdjson_PathTraversal_Nested(benchmark::State& state) {
    sj::dom::parser parser;
    sj::padded_string padded{ Dataset::Get().nested };
    sj::dom::element doc{ parser.parse(padded) };

    for (auto _ : state) {
        sj::dom::element cur = doc;
        for (int i{}; i < 10; ++i) {
            auto child{ cur["child"] };
            if (child.error()) break;
            cur = child.value_unsafe();
        }
        auto tag{ cur["meta"]["tag"].get_string() };
        benchmark::DoNotOptimize(tag);
    }
}

// ======================================================================
//  Registration
// ======================================================================

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

BENCH_PARSE_ALL(Simdjson);

// DOM variant
BENCHMARK_TEMPLATE(BM_Simdjson_DOM_Parse, DS::Medium)->Name(BENCH_NAME(Parse, Simdjson, DOM, Medium));
BENCHMARK_TEMPLATE(BM_Simdjson_DOM_Parse, DS::Large) ->Name(BENCH_NAME(Parse, Simdjson, DOM, Large));

BENCHMARK(BM_Simdjson_KeyAccess_Medium) ->Name(BENCH_NAME(KeyAccess, Simdjson, DOM, Medium));

BENCHMARK(BM_Simdjson_ArrayIteration_Array) ->Name(BENCH_NAME(ArrayIteration, Simdjson, DOM, Array));
BENCHMARK(BM_Simdjson_ArrayIteration_Large) ->Name(BENCH_NAME(ArrayIteration, Simdjson, DOM, Large));

BENCHMARK(BM_Simdjson_PathTraversal_Nested) ->Name(BENCH_NAME(PathTraversal, Simdjson, DOM, Nested));

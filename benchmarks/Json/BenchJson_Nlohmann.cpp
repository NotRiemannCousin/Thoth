/// @file BenchJson_Nlohmann.cpp
/// @brief JSON benchmarks for nlohmann/json (base + custom state allocator variant).

#include <benchmark/benchmark.h>

#include <nlohmann/json.hpp>

#include "BenchJson_Shared.hpp"

namespace nl = nlohmann;

// ======================================================================
//  NLOHMANN — Parse (base)
// ======================================================================

template<DS ds>
static void BM_Nlohmann_Parse(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    for (auto _ : state) {
        auto result{ nl::json::parse(src) };
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(src.size()));
    state.SetLabel(DSName(ds));
}

// ======================================================================
//  NLOHMANN — Stringify
// ======================================================================

template<DS ds>
static void BM_Nlohmann_Stringify(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    auto parsed{ nl::json::parse(src) };
    for (auto _ : state) {
        std::string out{ parsed.dump() };
        benchmark::DoNotOptimize(out);
    }
    state.SetLabel(DSName(ds));
}

// ======================================================================
//  NLOHMANN — Key Access
// ======================================================================

static void BM_Nlohmann_KeyAccess_Medium(benchmark::State& state) {
    auto parsed{ nl::json::parse(Dataset::Get().medium) };
    for (auto _ : state) {
        auto& total{ parsed["total"] };
        auto& page { parsed["page"]  };
        auto& users{ parsed["users"] };
        benchmark::DoNotOptimize(total);
        benchmark::DoNotOptimize(page);
        benchmark::DoNotOptimize(users);
    }
}

// ======================================================================
//  NLOHMANN — Array Iteration
// ======================================================================

static void BM_Nlohmann_ArrayIteration_Array(benchmark::State& state) {
    auto parsed{ nl::json::parse(Dataset::Get().array) };
    for (auto _ : state) {
        std::size_t count{};
        for (const auto& elem : parsed) {
            benchmark::DoNotOptimize(elem.at("label").get_ref<const std::string&>());
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
}

static void BM_Nlohmann_ArrayIteration_Large(benchmark::State& state) {
    auto parsed{ nl::json::parse(Dataset::Get().large) };
    for (auto _ : state) {
        std::size_t count{};
        for (const auto& elem : parsed["data"]) {
            benchmark::DoNotOptimize(elem.at("name").get_ref<const std::string&>());
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
}

// ======================================================================
//  NLOHMANN — Build Object
// ======================================================================

static void BM_Nlohmann_BuildObject(benchmark::State& state) {
    for (auto _ : state) {
        nl::json obj;
        obj["id"]     = 1;
        obj["name"]   = "Alice";
        obj["email"]  = "alice@example.com";
        obj["score"]  = 98.6;
        obj["active"] = true;
        obj["tags"]   = nl::json::array({"dev", "cpp", "bench"});
        obj["address"] = { {"street","123 Main St"}, {"city","Anytown"}, {"zip","12345"} };
        benchmark::DoNotOptimize(obj);
    }
}

// ======================================================================
//  NLOHMANN — Build Array
// ======================================================================

static void BM_Nlohmann_BuildArray(benchmark::State& state) {
    const int N = static_cast<int>(state.range(0));
    for (auto _ : state) {
        nl::json arr = nl::json::array();
        for (int i{}; i < N; ++i) {
            nl::json item;
            item["x"]     = i * 1.5;
            item["y"]     = i * -0.5;
            item["label"] = std::string("item_") + std::to_string(i);
            arr.push_back(std::move(item));
        }
        benchmark::DoNotOptimize(arr);
    }
}

// ======================================================================
//  NLOHMANN — Round-trip
// ======================================================================

static void BM_Nlohmann_RoundTrip_Medium(benchmark::State& state) {
    const std::string& src = Dataset::Get().medium;
    for (auto _ : state) {
        auto parsed{ nl::json::parse(src) };
        parsed["page"] = 2;
        std::string out{ parsed.dump() };
        benchmark::DoNotOptimize(out);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(src.size()));
}

// ======================================================================
//  NLOHMANN — Type Checking
// ======================================================================

static void BM_Nlohmann_TypeChecking(benchmark::State& state) {
    auto parsed{ nl::json::parse(Dataset::Get().medium) };
    const auto& users = parsed["users"];
    for (auto _ : state) {
        std::size_t numObj{}, numArr{}, numStr{}, numNum{}, numBool{};
        for (const auto& u : users) {
            if (u.is_object())  ++numObj;
            if (u.is_array())   ++numArr;
            if (u.is_string())  ++numStr;
            if (u.is_number())  ++numNum;
            if (u.is_boolean()) ++numBool;
        }
        benchmark::DoNotOptimize(numObj);
    }
}

// ======================================================================
//  NLOHMANN — Path Traversal
// ======================================================================

static void BM_Nlohmann_PathTraversal_Nested(benchmark::State& state) {
    auto parsed{ nl::json::parse(Dataset::Get().nested) };
    for (auto _ : state) {
        const nl::json* cur = &parsed;
        for (int i{}; i < 10; ++i) {
            auto it{ cur->find("child") };
            if (it == cur->end()) break;
            cur = &(*it);
        }
        std::string tag;
        auto metaIt{ cur->find("meta") };
        if (metaIt != cur->end()) {
            auto tagIt{ metaIt->find("tag") };
            if (tagIt != metaIt->end()) {
                tag = tagIt->get<std::string>();
            }
        }
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

#define BENCH_STR_DS(lib, DS_ENUM) \
    BENCHMARK_TEMPLATE(BM_##lib##_Stringify, DS::DS_ENUM)->Name(BENCH_NAME(Stringify, lib, Base, DS_ENUM))

BENCH_PARSE_ALL(Nlohmann);

BENCH_STR_DS(Nlohmann, Small);  BENCH_STR_DS(Nlohmann, Medium); BENCH_STR_DS(Nlohmann, Large);

BENCHMARK(BM_Nlohmann_KeyAccess_Medium) ->Name(BENCH_NAME(KeyAccess, Nlohmann, Base, Medium));

BENCHMARK(BM_Nlohmann_ArrayIteration_Array) ->Name(BENCH_NAME(ArrayIteration, Nlohmann, Base, Array));
BENCHMARK(BM_Nlohmann_ArrayIteration_Large) ->Name(BENCH_NAME(ArrayIteration, Nlohmann, Base, Large));

BENCHMARK(BM_Nlohmann_BuildObject) ->Name(BENCH_NAME(Build, Nlohmann, Base, Object));

BENCHMARK(BM_Nlohmann_BuildArray) ->Name(BENCH_NAME(Build, Nlohmann, Base, Array))->RangeMultiplier(4)->Range(10, 1000);

BENCHMARK(BM_Nlohmann_TypeChecking) ->Name(BENCH_NAME(TypeChecking, Nlohmann, Base, Medium));

BENCHMARK(BM_Nlohmann_PathTraversal_Nested) ->Name(BENCH_NAME(PathTraversal, Nlohmann, Base, Nested));

BENCHMARK(BM_Nlohmann_RoundTrip_Medium) ->Name(BENCH_NAME(RoundTrip, Nlohmann, Base, Medium));

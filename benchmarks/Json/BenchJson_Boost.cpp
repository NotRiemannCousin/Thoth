/// @file BenchJson_Boost.cpp
/// @brief JSON benchmarks for Boost.JSON (base + monotonic_resource variant).

#include <benchmark/benchmark.h>

#include <boost/json.hpp>

#include "BenchJson_Shared.hpp"

namespace bj = boost::json;

static constexpr bj::parse_options kParseOpts{
    .allow_comments      = false,
    .allow_trailing_commas = false,
};

// ======================================================================
//  BOOST — Parse (base: heap-stored)
// ======================================================================

template<DS ds>
static void BM_Boost_Parse(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    for (auto _ : state) {
        boost::system::error_code ec;
        auto val = bj::parse(src, ec, bj::storage_ptr{}, kParseOpts);
        benchmark::DoNotOptimize(val);
        benchmark::DoNotOptimize(ec);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(src.size()));
    state.SetLabel(DSName(ds));
}

// ======================================================================
//  BOOST — Parse (monotonic_resource: arena-backed)
// ======================================================================

template<DS ds>
static void BM_Boost_Parse_Mono(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    for (auto _ : state) {
        // Keep the value, storage_ptr and resource in one scope.  The value
        // owns references into the arena and must die before the resource.
        bj::monotonic_resource mr;
        bj::storage_ptr storage{&mr};
        boost::system::error_code ec;
        auto val = bj::parse(src, ec, storage, kParseOpts);
        benchmark::DoNotOptimize(val);
        benchmark::DoNotOptimize(ec);
        if (ec) {
            state.SkipWithError(ec.message().c_str());
            return;
        }
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(src.size()));
    state.SetLabel(std::string(DSName(ds)) + "/mono");
}

// ======================================================================
//  BOOST — Stringify
// ======================================================================

template<DS ds>
static void BM_Boost_Stringify(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    bj::value parsed{ bj::parse(src, bj::storage_ptr{}, kParseOpts) };
    for (auto _ : state) {
        std::string out{ bj::serialize(parsed) };
        benchmark::DoNotOptimize(out);
    }
    state.SetLabel(DSName(ds));
}

// ======================================================================
//  BOOST — Key Access
// ======================================================================

static void BM_Boost_KeyAccess_Medium(benchmark::State& state) {
    auto parsed{ bj::parse(Dataset::Get().medium, bj::storage_ptr{}, kParseOpts) };
    const auto& obj = parsed.as_object();
    for (auto _ : state) {
        const auto& total{ obj.at("total") };
        const auto& page { obj.at("page")  };
        const auto& users{ obj.at("users") };
        benchmark::DoNotOptimize(&total);
        benchmark::DoNotOptimize(&page);
        benchmark::DoNotOptimize(&users);
    }
}

// ======================================================================
//  BOOST — Array Iteration
// ======================================================================

static void BM_Boost_ArrayIteration_Array(benchmark::State& state) {
    auto parsed{ bj::parse(Dataset::Get().array, bj::storage_ptr{}, kParseOpts) };
    const auto& arr{ parsed.as_array() };
    for (auto _ : state) {
        std::size_t count{};
        for (const auto& elem : arr) {
            benchmark::DoNotOptimize(elem.as_object().at("label").as_string());
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
}

static void BM_Boost_ArrayIteration_Large(benchmark::State& state) {
    auto parsed{ bj::parse(Dataset::Get().large, bj::storage_ptr{}, kParseOpts) };
    const auto& root{ parsed.as_object() };
    const auto& arr{ root.at("data").as_array() };
    for (auto _ : state) {
        std::size_t count{};
        for (const auto& elem : arr) {
            benchmark::DoNotOptimize(elem.as_object().at("name").as_string());
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
}

// ======================================================================
//  BOOST — Build Object
// ======================================================================

static void BM_Boost_BuildObject(benchmark::State& state) {
    for (auto _ : state) {
        bj::object obj;
        obj["id"]     = 1;
        obj["name"]   = "Alice";
        obj["email"]  = "alice@example.com";
        obj["score"]  = 98.6;
        obj["active"] = true;
        obj["tags"]   = bj::value_from(std::vector<std::string>{"dev", "cpp", "bench"});
        obj["address"] = bj::value_from(std::map<std::string, std::string>{
            {"street", "123 Main St"}, {"city", "Anytown"}, {"zip", "12345"}
        });
        bj::value root{std::move(obj)};
        benchmark::DoNotOptimize(root);
    }
}

// ======================================================================
//  BOOST — Build Array
// ======================================================================

static void BM_Boost_BuildArray(benchmark::State& state) {
    const int N = static_cast<int>(state.range(0));
    for (auto _ : state) {
        bj::array arr;
        arr.reserve(N);
        for (int i{}; i < N; ++i) {
            bj::object item;
            item["x"]     = i * 1.5;
            item["y"]     = i * -0.5;
            item["label"] = std::string("item_") + std::to_string(i);
            arr.emplace_back(std::move(item));
        }
        bj::value root{std::move(arr)};
        benchmark::DoNotOptimize(root);
    }
}

// ======================================================================
//  BOOST — Round-trip
// ======================================================================

static void BM_Boost_RoundTrip_Medium(benchmark::State& state) {
    const std::string& src = Dataset::Get().medium;
    for (auto _ : state) {
        boost::system::error_code ec;
        auto parsed{ bj::parse(src, ec, bj::storage_ptr{}, kParseOpts) };
        if (ec) { state.SkipWithError("parse failed"); return; }
        parsed.as_object()["page"] = 2;
        std::string out{ bj::serialize(parsed) };
        benchmark::DoNotOptimize(out);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(src.size()));
}

// ======================================================================
//  BOOST — Type Checking
// ======================================================================

static void BM_Boost_TypeChecking(benchmark::State& state) {
    auto parsed{ bj::parse(Dataset::Get().medium, bj::storage_ptr{}, kParseOpts) };
    const auto& users = parsed.as_object().at("users").as_array();
    for (auto _ : state) {
        std::size_t numObj{}, numArr{}, numStr{}, numNum{}, numBool{};
        for (const auto& u : users) {
            if (u.is_object())  ++numObj;
            if (u.is_array())   ++numArr;
            if (u.is_string())  ++numStr;
            if (u.is_number())  ++numNum;
            if (u.is_bool())    ++numBool;
        }
        benchmark::DoNotOptimize(numObj);
    }
}

// ======================================================================
//  BOOST — Path Traversal
// ======================================================================

static void BM_Boost_PathTraversal_Nested(benchmark::State& state) {
    auto parsed{ bj::parse(Dataset::Get().nested, bj::storage_ptr{}, kParseOpts) };
    for (auto _ : state) {
        const bj::value* cur = &parsed;
        for (int i{}; i < 10; ++i) {
            const auto* obj = cur->if_object();
            if (!obj) break;
            auto it = obj->find("child");
            if (it == obj->end()) break;
            cur = &it->value();
        }
        std::string tag;
        const auto* obj = cur->if_object();
        if (obj) {
            auto metaIt = obj->find("meta");
            if (metaIt != obj->end()) {
                const auto* meta = metaIt->value().if_object();
                if (meta) {
                    auto tagIt = meta->find("tag");
                    if (tagIt != meta->end()) {
                        tag = tagIt->value().as_string().c_str();
                    }
                }
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

BENCH_PARSE_ALL(Boost);

BENCHMARK_TEMPLATE(BM_Boost_Parse_Mono, DS::Medium)->Name(BENCH_NAME(Parse, Boost, Monotonic, Medium));
BENCHMARK_TEMPLATE(BM_Boost_Parse_Mono, DS::Large) ->Name(BENCH_NAME(Parse, Boost, Monotonic, Large));

BENCH_STR_DS(Boost, Small);  BENCH_STR_DS(Boost, Medium); BENCH_STR_DS(Boost, Large);

BENCHMARK(BM_Boost_KeyAccess_Medium) ->Name(BENCH_NAME(KeyAccess, Boost, Base, Medium));

BENCHMARK(BM_Boost_ArrayIteration_Array) ->Name(BENCH_NAME(ArrayIteration, Boost, Base, Array));
BENCHMARK(BM_Boost_ArrayIteration_Large) ->Name(BENCH_NAME(ArrayIteration, Boost, Base, Large));

BENCHMARK(BM_Boost_BuildObject) ->Name(BENCH_NAME(Build, Boost, Base, Object));

BENCHMARK(BM_Boost_BuildArray) ->Name(BENCH_NAME(Build, Boost, Base, Array))->RangeMultiplier(4)->Range(10, 1000);

BENCHMARK(BM_Boost_TypeChecking) ->Name(BENCH_NAME(TypeChecking, Boost, Base, Medium));

BENCHMARK(BM_Boost_PathTraversal_Nested) ->Name(BENCH_NAME(PathTraversal, Boost, Base, Nested));
BENCHMARK(BM_Boost_RoundTrip_Medium) ->Name(BENCH_NAME(RoundTrip, Boost, Base, Medium));

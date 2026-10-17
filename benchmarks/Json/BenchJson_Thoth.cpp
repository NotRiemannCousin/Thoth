/// @file BenchJson_Thoth.cpp
/// @brief JSON benchmarks for Thoth::NJson (normal, zero-copy, and mimalloc modes).

#include <benchmark/benchmark.h>

#include <Thoth/NJson/Json.hpp>
#include <Thoth/NJson/JsonObject.hpp>

#include "BenchJson_Shared.hpp"

// Uncomment to test with mimalloc globally overriding new/delete:
// #include <mimalloc-new-delete.h>

namespace tj = Thoth::NJson;

using namespace std::string_literals;

// ======================================================================
//  THOTH — Parse
// ======================================================================

template<DS ds>
static void BM_Thoth_Parse(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    for (auto _ : state) {
        auto result{ tj::Json::Parse(src) };
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(src.size()));
    state.SetLabel(DSName(ds));
}

// Zero-copy: string_view into caller buffer, no internal heap copy.
template<DS ds>
static void BM_Thoth_Parse_NoCopy(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    for (auto _ : state) {
        auto result{ tj::Json::ParseText(src, false) };
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(src.size()));
    state.SetLabel(std::string(DSName(ds)) + "/nocopy");
}

// mimalloc variant: same body, linked against mimalloc-static.
// Only meaningful when this object file is linked with mimalloc before
// other allocator libs — creates a separate label for comparison.
template<DS ds>
static void BM_Thoth_Parse_Mimalloc(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    for (auto _ : state) {
        auto result{ tj::Json::Parse(src) };
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(src.size()));
    state.SetLabel(std::string(DSName(ds)) + "/mimalloc");
}

// ======================================================================
//  THOTH — Stringify
// ======================================================================

template<DS ds>
static void BM_Thoth_Stringify(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    auto parsed{ tj::Json::Parse(src) };
    if (!parsed) { state.SkipWithError("parse failed"); return; }

    for (auto _ : state) {
        std::string out{ std::format("{}", *parsed) };
        benchmark::DoNotOptimize(out);
    }
    state.SetLabel(DSName(ds));
}

// ======================================================================
//  THOTH — Key Access
// ======================================================================

static void BM_Thoth_KeyAccess_Medium(benchmark::State& state) {
    auto parsed{ tj::Json::Parse(Dataset::Get().medium) };
    if (!parsed) { state.SkipWithError("parse failed"); return; }

    for (auto _ : state) {
        auto total{ parsed->Get("total") };
        auto page { parsed->Get("page")  };
        auto users{ parsed->Get("users") };
        benchmark::DoNotOptimize(total);
        benchmark::DoNotOptimize(page);
        benchmark::DoNotOptimize(users);
    }
}

// ======================================================================
//  THOTH — Array Iteration
// ======================================================================

static void BM_Thoth_ArrayIteration_Array(benchmark::State& state) {
    auto parsed{ tj::Json::Parse(Dataset::Get().array) };
    if (!parsed) { state.SkipWithError("parse failed"); return; }

    for (auto _ : state) {
        std::size_t count{};
        for (const auto& elem : parsed->AsRef<tj::Array>()) {
            benchmark::DoNotOptimize(elem.Get("label"));
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
}

static void BM_Thoth_ArrayIteration_Large(benchmark::State& state) {
    auto parsed{ tj::Json::Parse(Dataset::Get().large) };
    if (!parsed) { state.SkipWithError("parse failed"); return; }

    for (auto _ : state) {
        std::size_t count{};
        for (const auto& elem : (*parsed->Get("data"))->AsRef<tj::Array>()) {
            benchmark::DoNotOptimize(elem.Get("name"));
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
}

// ======================================================================
//  THOTH — Path Traversal
// ======================================================================

static void BM_Thoth_PathTraversal_Nested(benchmark::State& state) {
    auto parsed{ tj::Json::Parse(Dataset::Get().nested) };
    if (!parsed) { state.SkipWithError("parse failed"); return; }

    static constexpr auto keys{ std::invoke([] {
        std::array<tj::KeyView, 12> arr{};
        for (int i{}; i < 10; ++i) arr[i] = "child";
        arr[10] = "meta";
        arr[11] = "tag";
        return arr;
    }) };

    for (auto _ : state) {
        auto tag{ parsed->Find(keys) };
        benchmark::DoNotOptimize(tag);
    }
}

// ======================================================================
//  THOTH — Build Object
// ======================================================================

static void BM_Thoth_BuildObject(benchmark::State& state) {
    using namespace tj;
    for (auto _ : state) {
        JsonObject obj;

        obj["id"]      = 1;
        obj["name"]    = "Alice";
        obj["email"]   = "alice@example.com";
        obj["score"]   = 98.6L;
        obj["active"]  = true;
        obj["tags"]    = Array{ "dev"s, "cpp"s, "bench"s };
        obj["address"] = JsonObject{
            { "street"s, "123 Main St"s },
            { "city"s,   "Anytown"s },
            { "zip"s,    "12345"s },
        };

        Json root{ std::move(obj) };
        benchmark::DoNotOptimize(root);
    }
}

// ======================================================================
//  THOTH — Build Array
// ======================================================================

static void BM_Thoth_BuildArray(benchmark::State& state) {
    using namespace tj;
    const int N{ static_cast<int>(state.range(0)) };
    for (auto _ : state) {
        Array arr;
        arr.reserve(static_cast<std::size_t>(N));
        for (int i{}; i < N; ++i) {
            JsonObject item{};
            item["x"]     = i * 1.5L;
            item["y"]     = i * -0.5L;
            item["label"] = "item_"s + std::to_string(i);
            arr.emplace_back(std::move(item));
        }
        Json root{ std::move(arr) };
        benchmark::DoNotOptimize(root);
    }
}

// ======================================================================
//  THOTH — Type Checking
// ======================================================================

static void BM_Thoth_TypeChecking(benchmark::State& state) {
    using namespace tj;
    auto parsed{ Json::Parse(Dataset::Get().medium) };
    if (!parsed) { state.SkipWithError("parse failed"); return; }

    auto usersOpt{ parsed->Get("users") };
    if (!usersOpt) { state.SkipWithError("no users"); return; }
    const auto& users{ (*usersOpt)->AsRef<Array>() };

    for (auto _ : state) {
        std::size_t numObj{}, numArr{}, numStr{}, numNum{}, numBool{};
        for (const Json& u : users) {
            if (u.IsOf<Object>())  ++numObj;
            if (u.IsOf<Array>())   ++numArr;
            if (u.IsOf<String>())  ++numStr;
            if (u.IsOf<Number>())  ++numNum;
            if (u.IsOf<Bool>())    ++numBool;
        }
        benchmark::DoNotOptimize(numObj);
    }
}

// ======================================================================
//  THOTH — Round-Trip
// ======================================================================

static void BM_Thoth_RoundTrip_Medium(benchmark::State& state) {
    const std::string& src{ Dataset::Get().medium };
    for (auto _ : state) {
        auto parsed{ tj::Json::Parse(src) };
        if (!parsed) { state.SkipWithError("parse failed"); return; }
        auto pageOpt{ parsed->Get("page") };
        if (pageOpt) **pageOpt = 2;

        std::string out{ std::format("{}", *parsed) };
        benchmark::DoNotOptimize(out);
    }
    state.SetBytesProcessed(
        static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(src.size()));
}

// ======================================================================
//  Registration macros (shared definitions, also used by other TUs)
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

// ======================================================================
//  THOTH — Registration
// ======================================================================

BENCH_PARSE_ALL(Thoth);

// Parse modifiers
BENCHMARK_TEMPLATE(BM_Thoth_Parse_NoCopy, DS::Medium)->Name(BENCH_NAME(Parse, Thoth, NoCopy, Medium));
BENCHMARK_TEMPLATE(BM_Thoth_Parse_NoCopy, DS::Large) ->Name(BENCH_NAME(Parse, Thoth, NoCopy, Large));
BENCHMARK_TEMPLATE(BM_Thoth_Parse_Mimalloc, DS::Medium)->Name(BENCH_NAME(Parse, Thoth, Mimalloc, Medium));
BENCHMARK_TEMPLATE(BM_Thoth_Parse_Mimalloc, DS::Large) ->Name(BENCH_NAME(Parse, Thoth, Mimalloc, Large));

// Stringify
BENCH_STR_DS(Thoth, Small);  BENCH_STR_DS(Thoth, Medium); BENCH_STR_DS(Thoth, Large);

// Key Access
BENCHMARK(BM_Thoth_KeyAccess_Medium)->Name(BENCH_NAME(KeyAccess, Thoth, Base, Medium));

// Array Iteration
BENCHMARK(BM_Thoth_ArrayIteration_Array)->Name(BENCH_NAME(ArrayIteration, Thoth, Base, Array));
BENCHMARK(BM_Thoth_ArrayIteration_Large)->Name(BENCH_NAME(ArrayIteration, Thoth, Base, Large));

// Build Object
BENCHMARK(BM_Thoth_BuildObject)->Name(BENCH_NAME(Build, Thoth, Base, Object));

// Build Array
BENCHMARK(BM_Thoth_BuildArray)->Name(BENCH_NAME(Build, Thoth, Base, Array))->RangeMultiplier(4)->Range(10, 1000);

// Type Checking
BENCHMARK(BM_Thoth_TypeChecking)->Name(BENCH_NAME(TypeChecking, Thoth, Base, Medium));

// Path Traversal
BENCHMARK(BM_Thoth_PathTraversal_Nested)->Name(BENCH_NAME(PathTraversal, Thoth, Base, Nested));

// Round-trip
BENCHMARK(BM_Thoth_RoundTrip_Medium)->Name(BENCH_NAME(RoundTrip, Thoth, Base, Medium));

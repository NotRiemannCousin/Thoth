/// @file BenchJson_Yyjson.cpp
/// @brief JSON benchmarks for yyjson (base + read-only variants).

extern "C" {
#include <yyjson.h>
}

#include <benchmark/benchmark.h>

#include <cstdlib>

#include "BenchJson_Shared.hpp"

// ======================================================================
//  YYJSON — Parse (base: mutable, owns everything)
// ======================================================================

template<DS ds>
static void BM_Yyjson_Parse(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    for (auto _ : state) {
        auto doc{ yyjson_read(src.data(), src.size(), 0) };
        benchmark::DoNotOptimize(doc);
        yyjson_doc_free(doc);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(src.size()));
    state.SetLabel(DSName(ds));
}

// ======================================================================
//  YYJSON — Parse (read-only: no null-terminator needed, no alloc for doc)
// ======================================================================

template<DS ds>
static void BM_Yyjson_Parse_ReadOnly(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    for (auto _ : state) {
        auto doc{ yyjson_read(src.data(), src.size(), 0) };
        benchmark::DoNotOptimize(doc);
        yyjson_doc_free(doc);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(src.size()));
    state.SetLabel(std::string(DSName(ds)) + "/readonly");
}

// ======================================================================
//  YYJSON — Stringify
// ======================================================================

template<DS ds>
static void BM_Yyjson_Stringify(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    auto doc{ yyjson_read(src.data(), src.size(), 0) };
    for (auto _ : state) {
        size_t len{};
        char* out{ yyjson_write(doc, 0, &len) };
        benchmark::DoNotOptimize(len);
        std::free(out);
    }
    state.SetLabel(DSName(ds));
    yyjson_doc_free(doc);
}

// ======================================================================
//  YYJSON — Key Access
// ======================================================================

static void BM_Yyjson_KeyAccess_Medium(benchmark::State& state) {
    auto doc{ yyjson_read(Dataset::Get().medium.data(),
                          Dataset::Get().medium.size(), 0) };
    auto root{ yyjson_doc_get_root(doc) };
    for (auto _ : state) {
        auto total{ yyjson_obj_get(root, "total") };
        auto page { yyjson_obj_get(root, "page")  };
        auto users{ yyjson_obj_get(root, "users") };
        benchmark::DoNotOptimize(total);
        benchmark::DoNotOptimize(page);
        benchmark::DoNotOptimize(users);
    }
    yyjson_doc_free(doc);
}

// ======================================================================
//  YYJSON — Array Iteration
// ======================================================================

static void BM_Yyjson_ArrayIteration_Array(benchmark::State& state) {
    auto doc{ yyjson_read(Dataset::Get().array.data(),
                          Dataset::Get().array.size(), 0) };
    auto root{ yyjson_doc_get_root(doc) };
    for (auto _ : state) {
        std::size_t count{};

        yyjson_arr_iter iter;
        yyjson_arr_iter_init(root, &iter);
        yyjson_val *elem;

        while ((elem = yyjson_arr_iter_next(&iter))) {
            benchmark::DoNotOptimize(yyjson_obj_get(elem, "label"));
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
    yyjson_doc_free(doc);
}

static void BM_Yyjson_ArrayIteration_Large(benchmark::State& state) {
    auto doc{ yyjson_read(Dataset::Get().large.data(),
                          Dataset::Get().large.size(), 0) };
    auto root{ yyjson_doc_get_root(doc) };
    auto dataArr{ yyjson_obj_get(root, "data") };
    for (auto _ : state) {
        std::size_t count{};

        yyjson_arr_iter iter;
        yyjson_arr_iter_init(dataArr, &iter);
        yyjson_val *elem;

        while ((elem = yyjson_arr_iter_next(&iter))) {
            benchmark::DoNotOptimize(yyjson_obj_get(elem, "name"));
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
    yyjson_doc_free(doc);
}


// ======================================================================
//  YYJSON — Build Object
// ======================================================================

static void BM_Yyjson_BuildObject(benchmark::State& state) {
    for (auto _ : state) {
        auto doc{ yyjson_mut_doc_new(nullptr) };
        auto root{ yyjson_mut_obj(doc) };
        yyjson_mut_doc_set_root(doc, root);

        yyjson_mut_obj_add_int (doc, root, "id",     1);
        yyjson_mut_obj_add_str (doc, root, "name",   "Alice");
        yyjson_mut_obj_add_str (doc, root, "email",  "alice@example.com");
        yyjson_mut_obj_add_real(doc, root, "score",  98.6);
        yyjson_mut_obj_add_bool(doc, root, "active", true);

        auto tags{ yyjson_mut_arr(doc) };
        yyjson_mut_obj_add_val(doc, root, "tags", tags);
        yyjson_mut_arr_add_str(doc, tags, "dev");
        yyjson_mut_arr_add_str(doc, tags, "cpp");
        yyjson_mut_arr_add_str(doc, tags, "bench");

        auto address{ yyjson_mut_obj(doc) };
        yyjson_mut_obj_add_val(doc, root, "address", address);
        yyjson_mut_obj_add_str(doc, address, "street", "123 Main St");
        yyjson_mut_obj_add_str(doc, address, "city",   "Anytown");
        yyjson_mut_obj_add_str(doc, address, "zip",    "12345");

        benchmark::DoNotOptimize(doc);
        yyjson_mut_doc_free(doc);
    }
}

// ======================================================================
//  YYJSON — Build Array
// ======================================================================

static void BM_Yyjson_BuildArray(benchmark::State& state) {
    const int N = static_cast<int>(state.range(0));
    for (auto _ : state) {
        auto doc{ yyjson_mut_doc_new(nullptr) };
        auto root{ yyjson_mut_arr(doc) };
        yyjson_mut_doc_set_root(doc, root);

        for (int i{}; i < N; ++i) {
            auto item{ yyjson_mut_obj(doc) };
            yyjson_mut_obj_add_real(doc, item, "x",     i * 1.5);
            yyjson_mut_obj_add_real(doc, item, "y",     i * -0.5);
            std::string label{ std::string("item_") + std::to_string(i) };
            yyjson_mut_obj_add_str(doc, item, "label", label.c_str());
            yyjson_mut_arr_add_val(root, item);
        }
        benchmark::DoNotOptimize(doc);
        yyjson_mut_doc_free(doc);
    }
}

// ======================================================================
//  YYJSON — Round-trip
// ======================================================================

static void BM_Yyjson_RoundTrip_Medium(benchmark::State& state) {
    const std::string& src = Dataset::Get().medium;
    for (auto _ : state) {
        auto parsed{ yyjson_read(src.data(), src.size(), 0) };
        auto doc{ yyjson_doc_mut_copy(parsed, nullptr) };
        yyjson_doc_free(parsed);
        auto root{ yyjson_mut_doc_get_root(doc) };
        yyjson_mut_obj_add_int(doc, root, "page", 2);

        size_t len{};
        char* out{ yyjson_mut_write(doc, 0, &len) };
        benchmark::DoNotOptimize(len);
        std::free(out);
        yyjson_mut_doc_free(doc);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(src.size()));
}

// ======================================================================
//  YYJSON — Type Checking
// ======================================================================

static void BM_Yyjson_TypeChecking(benchmark::State& state) {
    auto doc{ yyjson_read(Dataset::Get().medium.data(),
                          Dataset::Get().medium.size(), 0) };
    auto root{ yyjson_doc_get_root(doc) };
    auto users{ yyjson_obj_get(root, "users") };
    for (auto _ : state) {
        std::size_t numObj{}, numArr{}, numStr{}, numNum{}, numBool{};
        size_t max{ yyjson_arr_size(users) };
        for (size_t i{}; i < max; ++i) {
            auto u{ yyjson_arr_get(users, i) };
            if (yyjson_is_obj(u))   ++numObj;
            if (yyjson_is_arr(u))   ++numArr;
            if (yyjson_is_str(u))   ++numStr;
            if (yyjson_is_num(u))   ++numNum;
            if (yyjson_is_bool(u))  ++numBool;
        }
        benchmark::DoNotOptimize(numObj);
    }
    yyjson_doc_free(doc);
}

// ======================================================================
//  YYJSON — Path Traversal
// ======================================================================

static void BM_Yyjson_PathTraversal_Nested(benchmark::State& state) {
    auto doc{ yyjson_read(Dataset::Get().nested.data(),
                          Dataset::Get().nested.size(), 0) };
    auto cur{ yyjson_doc_get_root(doc) };
    for (auto _ : state) {
        for (int i{}; i < 10; ++i) {
            cur = yyjson_obj_get(cur, "child");
            if (!cur) break;
        }
        std::string_view tag;
        if (cur) {
            auto meta{ yyjson_obj_get(cur, "meta") };
            if (meta) {
                auto tagVal{ yyjson_obj_get(meta, "tag") };
                if (tagVal) tag = yyjson_get_str(tagVal);
            }
        }
        benchmark::DoNotOptimize(tag);
    }
    yyjson_doc_free(doc);
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

BENCH_PARSE_ALL(Yyjson);

BENCHMARK_TEMPLATE(BM_Yyjson_Parse_ReadOnly, DS::Medium)->Name(BENCH_NAME(Parse, Yyjson, ReadOnly, Medium));
BENCHMARK_TEMPLATE(BM_Yyjson_Parse_ReadOnly, DS::Large) ->Name(BENCH_NAME(Parse, Yyjson, ReadOnly, Large));

BENCH_STR_DS(Yyjson, Small);  BENCH_STR_DS(Yyjson, Medium); BENCH_STR_DS(Yyjson, Large);

BENCHMARK(BM_Yyjson_KeyAccess_Medium) ->Name(BENCH_NAME(KeyAccess, Yyjson, Base, Medium));

BENCHMARK(BM_Yyjson_ArrayIteration_Array) ->Name(BENCH_NAME(ArrayIteration, Yyjson, Base, Array));
BENCHMARK(BM_Yyjson_ArrayIteration_Large) ->Name(BENCH_NAME(ArrayIteration, Yyjson, Base, Large));

BENCHMARK(BM_Yyjson_BuildObject) ->Name(BENCH_NAME(Build, Yyjson, Base, Object));

BENCHMARK(BM_Yyjson_BuildArray) ->Name(BENCH_NAME(Build, Yyjson, Base, Array))->RangeMultiplier(4)->Range(10, 1000);

BENCHMARK(BM_Yyjson_TypeChecking) ->Name(BENCH_NAME(TypeChecking, Yyjson, Base, Medium));

BENCHMARK(BM_Yyjson_PathTraversal_Nested) ->Name(BENCH_NAME(PathTraversal, Yyjson, Base, Nested));
BENCHMARK(BM_Yyjson_RoundTrip_Medium) ->Name(BENCH_NAME(RoundTrip, Yyjson, Base, Medium));

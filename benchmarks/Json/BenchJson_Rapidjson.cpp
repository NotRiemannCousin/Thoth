/// @file BenchJson_Rapidjson.cpp
/// @brief JSON benchmarks for RapidJSON (base + in-situ variants).

#include <benchmark/benchmark.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>

#include "BenchJson_Shared.hpp"

namespace rj = rapidjson;

static constexpr unsigned kRapidJsonFlags = rj::kParseValidateEncodingFlag | rj::kParseFullPrecisionFlag;

// ======================================================================
//  RAPIDJSON — Parse
// ======================================================================

template<DS ds>
static void BM_Rapidjson_Parse(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    for (auto _ : state) {
        rj::Document doc;
        doc.Parse<kRapidJsonFlags>(src.data(), src.size());
        benchmark::DoNotOptimize(doc);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(src.size()));
    state.SetLabel(DSName(ds));
}

// ======================================================================
//  RAPIDJSON — Parse (in-situ, modifies the buffer)
// ======================================================================

template<DS ds>
static void BM_Rapidjson_Parse_InSitu(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    std::string mutable_src{ src };

    for (auto _ : state) {
        rj::Document doc;
        doc.ParseInsitu<kRapidJsonFlags>(mutable_src.data());
        benchmark::DoNotOptimize(doc);
        mutable_src = src;  // restore for next iteration
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(src.size()));
    state.SetLabel(std::string(DSName(ds)) + "/insitu");
}

// ======================================================================
//  RAPIDJSON — Stringify
// ======================================================================

template<DS ds>
static void BM_Rapidjson_Stringify(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    rj::Document doc;
    doc.Parse<kRapidJsonFlags>(src.data(), src.size());

    for (auto _ : state) {
        rj::StringBuffer buf;
        rj::Writer writer{ buf };
        doc.Accept(writer);
        benchmark::DoNotOptimize(buf.GetString());
    }
    state.SetLabel(DSName(ds));
}

// ======================================================================
//  RAPIDJSON — Key Access
// ======================================================================

static void BM_Rapidjson_KeyAccess_Medium(benchmark::State& state) {
    rj::Document doc;
    doc.Parse<kRapidJsonFlags>(Dataset::Get().medium.data(), Dataset::Get().medium.size());

    for (auto _ : state) {
        const auto& total{ doc["total"] };
        const auto& page { doc["page"]  };
        const auto& users{ doc["users"] };
        benchmark::DoNotOptimize(total);
        benchmark::DoNotOptimize(page);
        benchmark::DoNotOptimize(users);
    }
}

// ======================================================================
//  RAPIDJSON — Array Iteration
// ======================================================================

static void BM_Rapidjson_ArrayIteration_Array(benchmark::State& state) {
    rj::Document doc;
    doc.Parse<kRapidJsonFlags>(Dataset::Get().array.data(), Dataset::Get().array.size());

    for (auto _ : state) {
        std::size_t count{};
        for (const auto& elem : doc.GetArray()) {
            benchmark::DoNotOptimize(elem["label"].GetString());
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
}

static void BM_Rapidjson_ArrayIteration_Large(benchmark::State& state) {
    rj::Document doc;
    doc.Parse<kRapidJsonFlags>(Dataset::Get().large.data(), Dataset::Get().large.size());

    for (auto _ : state) {
        std::size_t count{};
        for (const auto& elem : doc["data"].GetArray()) {
            benchmark::DoNotOptimize(elem["name"].GetString());
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
}

// ======================================================================
//  RAPIDJSON — Build Object
// ======================================================================

static void BM_Rapidjson_BuildObject(benchmark::State& state) {
    for (auto _ : state) {
        rj::Document doc;
        doc.SetObject();
        auto& alloc{ doc.GetAllocator() };

        doc.AddMember("id",     1,                           alloc);
        doc.AddMember("name",   "Alice",                     alloc);
        doc.AddMember("email",  "alice@example.com",         alloc);
        doc.AddMember("score",  98.6,                        alloc);
        doc.AddMember("active", true,                        alloc);

        rj::Value tags{ rj::kArrayType };
        tags.PushBack("dev",   alloc);
        tags.PushBack("cpp",   alloc);
        tags.PushBack("bench", alloc);
        doc.AddMember("tags", tags, alloc);

        rj::Value address{ rj::kObjectType };
        address.AddMember("street", "123 Main St", alloc);
        address.AddMember("city",   "Anytown",     alloc);
        address.AddMember("zip",    "12345",        alloc);
        doc.AddMember("address", address, alloc);

        benchmark::DoNotOptimize(doc);
    }
}

// ======================================================================
//  RAPIDJSON — Build Array
// ======================================================================

static void BM_Rapidjson_BuildArray(benchmark::State& state) {
    const int N{ static_cast<int>(state.range(0)) };
    for (auto _ : state) {
        rj::Document doc;
        doc.SetArray();
        auto& alloc{ doc.GetAllocator() };
        doc.Reserve(static_cast<rj::SizeType>(N), alloc);

        for (int i{}; i < N; ++i) {
            rj::Value item{ rj::kObjectType };
            item.AddMember("x",     i * 1.5,  alloc);
            item.AddMember("y",     i * -0.5, alloc);
            std::string label{ std::string("item_") + std::to_string(i) };
            rj::Value lv{ label.c_str(),
                static_cast<rj::SizeType>(label.size()), alloc };
            item.AddMember("label", lv, alloc);
            doc.PushBack(item, alloc);
        }
        benchmark::DoNotOptimize(doc);
    }
}

// ======================================================================
//  RAPIDJSON — Round-trip
// ======================================================================

static void BM_Rapidjson_RoundTrip_Medium(benchmark::State& state) {
    const std::string& src{ Dataset::Get().medium };
    for (auto _ : state) {
        rj::Document doc;
        doc.Parse<kRapidJsonFlags>(src.data(), src.size());
        doc["page"] = 2;
        rj::StringBuffer buf;
        rj::Writer<rj::StringBuffer> writer{ buf };
        doc.Accept(writer);
        benchmark::DoNotOptimize(buf.GetString());
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(src.size()));
}

// ======================================================================
//  RAPIDJSON — Path Traversal
// ======================================================================

static void BM_Rapidjson_PathTraversal_Nested(benchmark::State& state) {
    rj::Document doc;
    doc.Parse<kRapidJsonFlags>(Dataset::Get().nested.data(), Dataset::Get().nested.size());

    for (auto _ : state) {
        const rj::Value* cur{ &doc };
        for (int i{}; i < 10; ++i) {
            auto it{ cur->FindMember("child") };
            if (it == cur->MemberEnd()) break;
            cur = &it->value;
        }
        std::string_view tag;
        auto metaIt{ cur->FindMember("meta") };
        if (metaIt != cur->MemberEnd()) {
            auto tagIt{ metaIt->value.FindMember("tag") };
            if (tagIt != metaIt->value.MemberEnd())
                tag = tagIt->value.GetString();
        }
        benchmark::DoNotOptimize(tag);
    }
}

// ======================================================================
//  RAPIDJSON — Type Checking
// ======================================================================

static void BM_Rapidjson_TypeChecking(benchmark::State& state) {
    rj::Document doc;
    doc.Parse<kRapidJsonFlags>(Dataset::Get().medium.data(), Dataset::Get().medium.size());
    const auto& users = doc["users"].GetArray();

    for (auto _ : state) {
        std::size_t numObj{}, numArr{}, numStr{}, numNum{}, numBool{};
        for (const auto& u : users) {
            if (u.IsObject())  ++numObj;
            if (u.IsArray())   ++numArr;
            if (u.IsString())  ++numStr;
            if (u.IsNumber())  ++numNum;
            if (u.IsBool())    ++numBool;
        }
        benchmark::DoNotOptimize(numObj);
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

BENCH_PARSE_ALL(Rapidjson);

BENCHMARK_TEMPLATE(BM_Rapidjson_Parse_InSitu, DS::Medium)->Name(BENCH_NAME(Parse, Rapidjson, InSitu, Medium));
BENCHMARK_TEMPLATE(BM_Rapidjson_Parse_InSitu, DS::Large) ->Name(BENCH_NAME(Parse, Rapidjson, InSitu, Large));

BENCH_STR_DS(Rapidjson, Small);  BENCH_STR_DS(Rapidjson, Medium); BENCH_STR_DS(Rapidjson, Large);

BENCHMARK(BM_Rapidjson_KeyAccess_Medium) ->Name(BENCH_NAME(KeyAccess, Rapidjson, Base, Medium));

BENCHMARK(BM_Rapidjson_ArrayIteration_Array) ->Name(BENCH_NAME(ArrayIteration, Rapidjson, Base, Array));
BENCHMARK(BM_Rapidjson_ArrayIteration_Large) ->Name(BENCH_NAME(ArrayIteration, Rapidjson, Base, Large));

BENCHMARK(BM_Rapidjson_BuildObject) ->Name(BENCH_NAME(Build, Rapidjson, Base, Object));

BENCHMARK(BM_Rapidjson_BuildArray) ->Name(BENCH_NAME(Build, Rapidjson, Base, Array))->RangeMultiplier(4)->Range(10, 1000);

BENCHMARK(BM_Rapidjson_TypeChecking) ->Name(BENCH_NAME(TypeChecking, Rapidjson, Base, Medium));

BENCHMARK(BM_Rapidjson_PathTraversal_Nested) ->Name(BENCH_NAME(PathTraversal, Rapidjson, Base, Nested));
BENCHMARK(BM_Rapidjson_RoundTrip_Medium) ->Name(BENCH_NAME(RoundTrip, Rapidjson, Base, Medium));

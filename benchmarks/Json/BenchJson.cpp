/// AI GENERATED

/**
 * @file BenchJson.cpp
 * @brief Comprehensive JSON benchmarks comparing Thoth::NJson against
 *        nlohmann/json, simdjson, and RapidJSON.
 *
 * Build type matrix:
 *   - Parse             : string → DOM (all libs)
 *   - ParseNoCopy       : zero-copy / on-demand where supported
 *   - Stringify         : DOM → string
 *   - KeyAccess         : random key look-up on parsed object
 *   - ArrayIteration    : walk every element of a parsed array
 *   - BuildObject       : programmatic construction of a complex object
 *   - BuildArray        : programmatic construction of a large array
 *   - RoundTrip         : Parse → modify one key → Stringify
 *   - TypeChecking      : IsOf / type() calls on a prebuilt tree
 *   - PathTraversal     : multi-level key drill-down
 *
 * Dataset sizes:
 *   small   ~150 B   – single flat object
 *   medium  ~14 KB   – array of 40 user objects with nested address
 *   large   ~5 MB    – 10 000-element array from the VsNlohmann example
 *   nested  ~15 KB   – 20-level deep tree
 *   numbers ~5 KB    – 20×20 float matrix
 *   array   ~15 KB   – 200-element flat-ish array
 *   strings ~4 KB    – Unicode + escaped text
 */


#include <benchmark/benchmark.h>

// ── Thoth ─────────────────────────────────────────────────────────────
#include <Thoth/NJson/Json.hpp>
#include <Thoth/NJson/JsonObject.hpp>

// ── nlohmann ──────────────────────────────────────────────────────────
#include <nlohmann/json.hpp>

// ── simdjson ──────────────────────────────────────────────────────────
#include <simdjson.h>

// ── RapidJSON ─────────────────────────────────────────────────────────
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/error/en.h>

// ── std ───────────────────────────────────────────────────────────────
#include <fstream>
#include <sstream>
#include <string>
#include <format>
#include <print>
#include <filesystem>
#include <stdexcept>


namespace fs = std::filesystem;

// ======================================================================
//  Helpers
// ======================================================================

static std::string ReadFile(const char* rel) {
    const fs::path base{ BENCH_DATA_DIR };
    const fs::path full{ base / rel };
    std::ifstream f{ full, std::ios::binary };
    if (!f)
        throw std::runtime_error{ std::string("Cannot open: ") + full.string() };
    return { std::istreambuf_iterator<char>{f}, {} };
}

// Lazy-loaded, process-wide datasets
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
        static Dataset d = [] {
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

// Selector so benchmark functions can pick a dataset by enum
enum class DS { Small, Medium, Large, Nested, Numbers, Array, Strings, RawStrings, Twitter };

static const std::string& Pick(DS ds) {
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

static constexpr const char* DSName(DS ds) {
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
//  ██████╗  █████╗ ██████╗ ███████╗███████╗
//  ██╔══██╗██╔══██╗██╔══██╗██╔════╝██╔════╝
//  ██████╔╝███████║██████╔╝███████╗█████╗
//  ██╔═══╝ ██╔══██║██╔══██╗╚════██║██╔══╝
//  ██║     ██║  ██║██║  ██║███████║███████╗
//  ╚═╝     ╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚══════╝
// ======================================================================

// ── Parse ──────────────────────────────────────────────────────────────

template<DS ds>
static void BM_Thoth_Parse(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    for (auto _ : state) {
        auto result{ Thoth::NJson::Json::Parse(src) };
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(src.size()));
    state.SetLabel(DSName(ds));
}

// Zero-copy variant: Thoth keeps a string_view into the caller's buffer.
// Only safe while `src` is alive. Avoids an internal heap copy.
template<DS ds>
static void BM_Thoth_Parse_NoCopy(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    for (auto _ : state) {
        auto result{ Thoth::NJson::Json::ParseText(src, false) };
        benchmark::DoNotOptimize(result);
    }
    state.SetBytesProcessed(state.iterations() * static_cast<int64_t>(src.size()));
    state.SetLabel(std::string(DSName(ds)) + "/nocopy");
}

// ── Stringify ──────────────────────────────────────────────────────────

template<DS ds>
static void BM_Thoth_Stringify(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    auto parsed{ Thoth::NJson::Json::Parse(src) };
    if (!parsed) { state.SkipWithError("parse failed"); return; }

    for (auto _ : state) {
        std::string out{ std::format("{}", *parsed) };
        benchmark::DoNotOptimize(out);
    }
    state.SetLabel(DSName(ds));
}

// ── Key Access ─────────────────────────────────────────────────────────

static void BM_Thoth_KeyAccess_Medium(benchmark::State& state) {
    // medium.json: {"users": [...], "total": N, "page": 1}
    auto parsed{ Thoth::NJson::Json::Parse(Dataset::Get().medium) };
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

// ── Array Iteration ────────────────────────────────────────────────────

static void BM_Thoth_ArrayIteration_Array(benchmark::State& state) {
    // array.json: [{x,y,label,enabled}, ...]
    auto parsed{ Thoth::NJson::Json::Parse(Dataset::Get().array) };
    if (!parsed) { state.SkipWithError("parse failed"); return; }

    for (auto _ : state) {
        std::size_t count{};
        for (const auto& elem : parsed->AsRef<Thoth::NJson::Array>()) {
            benchmark::DoNotOptimize(elem.Get("label"));
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
}

static void BM_Thoth_ArrayIteration_Large(benchmark::State& state) {
    auto parsed{ Thoth::NJson::Json::Parse(Dataset::Get().large) };
    if (!parsed) { state.SkipWithError("parse failed"); return; }

    for (auto _ : state) {
        std::size_t count{};
        // large.json root is an object; "data" key holds the array

        for (const auto& elem : (*parsed->Get("data"))->AsRef<Thoth::NJson::Array>()) {
            benchmark::DoNotOptimize(elem.Get("name"));
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
}

// ── Path Traversal ─────────────────────────────────────────────────────

static void BM_Thoth_PathTraversal_Nested(benchmark::State& state) {
    // nested.json: { level:20, child:{ level:19, child:{...}}}
    // Drill 10 levels and read "meta.tag" at that node
    auto parsed{ Thoth::NJson::Json::Parse(Dataset::Get().nested) };
    if (!parsed) { state.SkipWithError("parse failed"); return; }

    std::vector<Thoth::NJson::Key> keys(10, "child");
    keys.emplace_back("meta");
    keys.emplace_back("tag");

    for (auto _ : state) {
        auto tag{ parsed->Find(keys) };
        benchmark::DoNotOptimize(tag);
    }
}

// ── Build Object ───────────────────────────────────────────────────────

static void BM_Thoth_BuildObject(benchmark::State& state) {
    using namespace Thoth::NJson;
    for (auto _ : state) {
        JsonObject obj;

        obj["id"]         = 1;
        obj["name"]       = "Alice";
        obj["email"]      = "alice@example.com";
        obj["score"]      = 98.6L;
        obj["active"]     = true;

        obj["tags"]       = Array{ "dev", "cpp", "bench" };


        obj["address"]    = JsonObject{
            { "street" , "123 Main St" },
            { "city"   , "Anytown" },
            { "zip"    , "12345" },
        };

        Json root{ std::move(obj) };
        benchmark::DoNotOptimize(root);
    }
}

// ── Build Array ────────────────────────────────────────────────────────

static void BM_Thoth_BuildArray(benchmark::State& state) {
    using namespace Thoth::NJson;
    const int N{ static_cast<int>(state.range(0)) };
    for (auto _ : state) {
        Array arr;
        arr.reserve(static_cast<std::size_t>(N));
        for (int i{}; i < N; ++i) {
            JsonObject item{};
            item["x"]     = i * 1.5L;
            item["y"]     = i * -0.5L;
            item["label"] = std::string("item_") + std::to_string(i);
            arr.emplace_back(std::move(item));
        }
        Json root{ std::move(arr) };
        benchmark::DoNotOptimize(root);
    }
}

// ── Type Checking ──────────────────────────────────────────────────────

static void BM_Thoth_TypeChecking(benchmark::State& state) {
    using namespace Thoth::NJson;
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

// ── Round-Trip ─────────────────────────────────────────────────────────

static void BM_Thoth_RoundTrip_Medium(benchmark::State& state) {
    const std::string& src{ Dataset::Get().medium };
    for (auto _ : state) {
        auto parsed{ Thoth::NJson::Json::Parse(src) };
        if (!parsed) { state.SkipWithError("parse failed"); return; }
        // modify one field to touch the tree
        auto pageOpt{ parsed->Get("page") };
        if (pageOpt) **pageOpt = 2;

        std::string out{ std::format("{}", *parsed) };
        benchmark::DoNotOptimize(out);
    }
    state.SetBytesProcessed(
        static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(src.size()));
}

// ======================================================================
//  NLOHMANN
// ======================================================================

template<DS ds>
static void BM_Nlohmann_Parse(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    for (auto _ : state) {
        auto result{ nlohmann::json::parse(src) };
        benchmark::DoNotOptimize(result);
    }

    state.SetBytesProcessed(
        static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(src.size()));
    state.SetLabel(DSName(ds));
}

template<DS ds>
static void BM_Nlohmann_Stringify(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    auto parsed{ nlohmann::json::parse(src) };
    for (auto _ : state) {
        std::string out = parsed.dump();
        benchmark::DoNotOptimize(out);
    }
    state.SetLabel(DSName(ds));
}

static void BM_Nlohmann_KeyAccess_Medium(benchmark::State& state) {
    auto parsed{ nlohmann::json::parse(Dataset::Get().medium) };
    for (auto _ : state) {
        auto& total{ parsed["total"] };
        auto& page { parsed["page"]  };
        auto& users{ parsed["users"] };
        benchmark::DoNotOptimize(total);
        benchmark::DoNotOptimize(page);
        benchmark::DoNotOptimize(users);
    }
}

static void BM_Nlohmann_ArrayIteration_Array(benchmark::State& state) {
    auto parsed{ nlohmann::json::parse(Dataset::Get().array) };
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
    auto parsed{ nlohmann::json::parse(Dataset::Get().large) };
    for (auto _ : state) {
        std::size_t count{};
        for (const auto& elem : parsed["data"]) {
            benchmark::DoNotOptimize(elem.at("name").get_ref<const std::string&>());
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
}

static void BM_Nlohmann_BuildObject(benchmark::State& state) {
    for (auto _ : state) {
        nlohmann::json obj;
        obj["id"]     = 1;
        obj["name"]   = "Alice";
        obj["email"]  = "alice@example.com";
        obj["score"]  = 98.6;
        obj["active"] = true;
        obj["tags"]   = nlohmann::json::array({"dev", "cpp", "bench"});
        obj["address"] = { {"street","123 Main St"}, {"city","Anytown"}, {"zip","12345"} };
        benchmark::DoNotOptimize(obj);
    }
}

static void BM_Nlohmann_BuildArray(benchmark::State& state) {
    const int N = static_cast<int>(state.range(0));
    for (auto _ : state) {
        nlohmann::json arr = nlohmann::json::array();
        for (int i{}; i < N; ++i) {
            nlohmann::json item;
            item["x"]     = i * 1.5;
            item["y"]     = i * -0.5;
            item["label"] = std::string("item_") + std::to_string(i);
            arr.push_back(std::move(item));
        }
        benchmark::DoNotOptimize(arr);
    }
}

static void BM_Nlohmann_RoundTrip_Medium(benchmark::State& state) {
    const std::string& src = Dataset::Get().medium;
    for (auto _ : state) {
        auto parsed{ nlohmann::json::parse(src) };
        parsed["page"] = 2;
        std::string out = parsed.dump();
        benchmark::DoNotOptimize(out);
    }
    state.SetBytesProcessed(
        static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(src.size()));
}

static void BM_Nlohmann_TypeChecking(benchmark::State& state) {
    auto parsed{ nlohmann::json::parse(Dataset::Get().medium) };
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

static void BM_Nlohmann_PathTraversal_Nested(benchmark::State& state) {
    auto parsed{ nlohmann::json::parse(Dataset::Get().nested) };
    for (auto _ : state) {
        const nlohmann::json* cur = &parsed;
        for (int i{}; i < 10; ++i) {
            auto it = cur->find("child");
            if (it == cur->end()) break;
            cur = &(*it);
        }
        std::string tag;
        auto metaIt{ cur->find("meta") };
        if (metaIt != cur->end()) {
            auto tagIt = metaIt->find("tag");
            if (tagIt != metaIt->end()) {
                tag = tagIt->get<std::string>();
            }
        }
        benchmark::DoNotOptimize(tag);
    }
}

// ======================================================================
//  SIMDJSON
//  simdjson uses a padded_string and a streaming on-demand parser;
//  the parser itself is reusable and should be declared outside the loop.
// ======================================================================

// Note: simdjson's on-demand API does NOT build a DOM tree eagerly —
// it's fundamentally a lazy-scan approach, so "parse" is fast but
// random access is less ergonomic. We benchmark the realistic pattern.

template<DS ds>
static void BM_Simdjson_Parse(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    simdjson::ondemand::parser parser;
    simdjson::padded_string padded{ src };

    for (auto _ : state) {
        simdjson::ondemand::document doc = parser.iterate(padded);
        // Force materialisation of the root value so the parser does real work
        auto val{ doc.get_value() };
        benchmark::DoNotOptimize(val);
    }
    state.SetBytesProcessed(
        static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(src.size()));
    state.SetLabel(DSName(ds));
}

// DOM-style parse (simdjson DOM API — the older, fully-materialised API)
template<DS ds>
static void BM_Simdjson_DOM_Parse(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    simdjson::dom::parser parser;
    simdjson::padded_string padded{ src };

    for (auto _ : state) {
        simdjson::dom::element doc = parser.parse(padded);
        benchmark::DoNotOptimize(doc);
    }
    state.SetBytesProcessed(
        static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(src.size()));
    state.SetLabel(std::string(DSName(ds)) + "/dom");
}

static void BM_Simdjson_KeyAccess_Medium(benchmark::State& state) {
    simdjson::dom::parser parser;
    simdjson::padded_string padded{ Dataset::Get().medium };
    simdjson::dom::element doc = parser.parse(padded);

    for (auto _ : state) {
        auto total{ doc["total"].get_int64() };
        auto page { doc["page"].get_int64()  };
        auto users{ doc["users"].get_array() };
        benchmark::DoNotOptimize(total);
        benchmark::DoNotOptimize(page);
        benchmark::DoNotOptimize(users);
    }
}

static void BM_Simdjson_ArrayIteration_Array(benchmark::State& state) {
    simdjson::dom::parser parser;
    simdjson::padded_string padded{ Dataset::Get().array };
    simdjson::dom::element doc = parser.parse(padded);

    for (auto _ : state) {
        std::size_t count{};
        for (simdjson::dom::element elem : doc.get_array()) {
            auto label{ elem["label"].get_string() };
            benchmark::DoNotOptimize(label);
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
}

static void BM_Simdjson_ArrayIteration_Large(benchmark::State& state) {
    simdjson::dom::parser parser;
    simdjson::padded_string padded{ Dataset::Get().large };
    simdjson::dom::element doc = parser.parse(padded);

    for (auto _ : state) {
        std::size_t count{};
        for (simdjson::dom::element elem : doc["data"].get_array()) {
            auto name{ elem["name"].get_string() };
            benchmark::DoNotOptimize(name);
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
}

static void BM_Simdjson_PathTraversal_Nested(benchmark::State& state) {
    simdjson::dom::parser parser;
    simdjson::padded_string padded{ Dataset::Get().nested };
    simdjson::dom::element doc{ parser.parse(padded) };

    for (auto _ : state) {
        simdjson::dom::element cur = doc;
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
//  RAPIDJSON
// ======================================================================

template<DS ds>
static void BM_Rapidjson_Parse(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    for (auto _ : state) {
        rapidjson::Document doc;
        doc.Parse(src.data(), src.size());
        benchmark::DoNotOptimize(doc);
    }
    state.SetBytesProcessed(
        static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(src.size()));
    state.SetLabel(DSName(ds));
}

// In-situ parse: RapidJSON modifies the input buffer in place (faster)
template<DS ds>
static void BM_Rapidjson_Parse_InSitu(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    std::string mutable_src{ src };

    for (auto _ : state) {
        rapidjson::Document doc;
        doc.ParseInsitu(mutable_src.data());
        benchmark::DoNotOptimize(doc);
        mutable_src = src;
    }
    state.SetBytesProcessed(
        static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(src.size()));
    state.SetLabel(std::string(DSName(ds)) + "/insitu");
}

template<DS ds>
static void BM_Rapidjson_Stringify(benchmark::State& state) {
    const std::string& src{ Pick(ds) };
    rapidjson::Document doc;
    doc.Parse(src.data(), src.size());

    for (auto _ : state) {
        rapidjson::StringBuffer buf;
        rapidjson::Writer writer{ buf };
        doc.Accept(writer);
        benchmark::DoNotOptimize(buf.GetString());
    }
    state.SetLabel(DSName(ds));
}

static void BM_Rapidjson_KeyAccess_Medium(benchmark::State& state) {
    rapidjson::Document doc;
    doc.Parse(Dataset::Get().medium.data(), Dataset::Get().medium.size());

    for (auto _ : state) {
        const auto& total{ doc["total"] };
        const auto& page { doc["page"]  };
        const auto& users{ doc["users"] };
        benchmark::DoNotOptimize(total);
        benchmark::DoNotOptimize(page);
        benchmark::DoNotOptimize(users);
    }
}

static void BM_Rapidjson_ArrayIteration_Array(benchmark::State& state) {
    rapidjson::Document doc;
    doc.Parse(Dataset::Get().array.data(), Dataset::Get().array.size());

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
    rapidjson::Document doc;
    doc.Parse(Dataset::Get().large.data(), Dataset::Get().large.size());

    for (auto _ : state) {
        std::size_t count{};
        for (const auto& elem : doc["data"].GetArray()) {
            benchmark::DoNotOptimize(elem["name"].GetString());
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }
}

static void BM_Rapidjson_BuildObject(benchmark::State& state) {
    for (auto _ : state) {
        rapidjson::Document doc;
        doc.SetObject();
        auto& alloc{ doc.GetAllocator() };

        doc.AddMember("id",     1,                           alloc);
        doc.AddMember("name",   "Alice",                     alloc);
        doc.AddMember("email",  "alice@example.com",         alloc);
        doc.AddMember("score",  98.6,                        alloc);
        doc.AddMember("active", true,                        alloc);

        rapidjson::Value tags{ rapidjson::kArrayType };
        tags.PushBack("dev",   alloc);
        tags.PushBack("cpp",   alloc);
        tags.PushBack("bench", alloc);
        doc.AddMember("tags", tags, alloc);

        rapidjson::Value address{ rapidjson::kObjectType };
        address.AddMember("street", "123 Main St", alloc);
        address.AddMember("city",   "Anytown",     alloc);
        address.AddMember("zip",    "12345",        alloc);
        doc.AddMember("address", address, alloc);

        benchmark::DoNotOptimize(doc);
    }
}

static void BM_Rapidjson_BuildArray(benchmark::State& state) {
    const int N{ static_cast<int>(state.range(0)) };
    for (auto _ : state) {
        rapidjson::Document doc;
        doc.SetArray();
        auto& alloc{ doc.GetAllocator() };
        doc.Reserve(static_cast<rapidjson::SizeType>(N), alloc);

        for (int i{}; i < N; ++i) {
            rapidjson::Value item{ rapidjson::kObjectType };
            item.AddMember("x",     i * 1.5,  alloc);
            item.AddMember("y",     i * -0.5, alloc);
            std::string label{ std::string("item_") + std::to_string(i) };
            rapidjson::Value lv{ label.c_str(),
                static_cast<rapidjson::SizeType>(label.size()), alloc };
            item.AddMember("label", lv, alloc);
            doc.PushBack(item, alloc);
        }
        benchmark::DoNotOptimize(doc);
    }
}

static void BM_Rapidjson_RoundTrip_Medium(benchmark::State& state) {
    const std::string& src{ Dataset::Get().medium };
    for (auto _ : state) {
        rapidjson::Document doc;
        doc.Parse(src.data(), src.size());
        doc["page"] = 2;
        rapidjson::StringBuffer buf;
        rapidjson::Writer<rapidjson::StringBuffer> writer{ buf };
        doc.Accept(writer);
        benchmark::DoNotOptimize(buf.GetString());
    }
    state.SetBytesProcessed(
        static_cast<int64_t>(state.iterations()) * static_cast<int64_t>(src.size()));
}

static void BM_Rapidjson_PathTraversal_Nested(benchmark::State& state) {
    rapidjson::Document doc;
    doc.Parse(Dataset::Get().nested.data(), Dataset::Get().nested.size());

    for (auto _ : state) {
        const rapidjson::Value* cur{ &doc };
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

static void BM_Rapidjson_TypeChecking(benchmark::State& state) {
    rapidjson::Document doc;
    doc.Parse(Dataset::Get().medium.data(), Dataset::Get().medium.size());
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
//  Registration macros
// ======================================================================

// ── Parse – all libs, all datasets ─────────────────────────────────────
#define BENCH_PARSE_DS(lib, DS_ENUM) \
    BENCHMARK_TEMPLATE(BM_##lib##_Parse, DS::DS_ENUM)->Name("Parse/" #lib "/" #DS_ENUM)

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

BENCH_PARSE_ALL(Thoth);
BENCH_PARSE_ALL(Nlohmann);
BENCH_PARSE_ALL(Simdjson);
BENCH_PARSE_ALL(Rapidjson);

// ── Parse NoCopy / InSitu ──────────────────────────────────────────────
BENCHMARK_TEMPLATE(BM_Thoth_Parse_NoCopy,     DS::Medium)->Name("Parse/Thoth/Medium/NoCopy");
BENCHMARK_TEMPLATE(BM_Thoth_Parse_NoCopy,     DS::Large) ->Name("Parse/Thoth/Large/NoCopy");
BENCHMARK_TEMPLATE(BM_Simdjson_DOM_Parse,     DS::Medium)->Name("Parse/Simdjson_DOM/Medium");
BENCHMARK_TEMPLATE(BM_Simdjson_DOM_Parse,     DS::Large) ->Name("Parse/Simdjson_DOM/Large");
BENCHMARK_TEMPLATE(BM_Rapidjson_Parse_InSitu, DS::Medium)->Name("Parse/Rapidjson/Medium/InSitu");
BENCHMARK_TEMPLATE(BM_Rapidjson_Parse_InSitu, DS::Large) ->Name("Parse/Rapidjson/Large/InSitu");

// ── Stringify ──────────────────────────────────────────────────────────
#define BENCH_STR_DS(lib, DS_ENUM) \
    BENCHMARK_TEMPLATE(BM_##lib##_Stringify, DS::DS_ENUM)->Name("Stringify/" #lib "/" #DS_ENUM)

BENCH_STR_DS(Thoth,    Small);  BENCH_STR_DS(Thoth,    Medium); BENCH_STR_DS(Thoth,    Large);
BENCH_STR_DS(Nlohmann, Small);  BENCH_STR_DS(Nlohmann, Medium); BENCH_STR_DS(Nlohmann, Large);
BENCH_STR_DS(Rapidjson,Small);  BENCH_STR_DS(Rapidjson,Medium); BENCH_STR_DS(Rapidjson,Large);
// BENCH_STR_DS(Thoth,    RawStrings);
// BENCH_STR_DS(Nlohmann, RawStrings);
// BENCH_STR_DS(Rapidjson,RawStrings);
// BENCH_STR_DS(Thoth,    Strings);
// BENCH_STR_DS(Nlohmann, Strings);
// BENCH_STR_DS(Rapidjson,Strings);

// ── Key Access ─────────────────────────────────────────────────────────
BENCHMARK(BM_Thoth_KeyAccess_Medium)    ->Name("KeyAccess/Thoth/Medium");
BENCHMARK(BM_Nlohmann_KeyAccess_Medium) ->Name("KeyAccess/Nlohmann/Medium");
BENCHMARK(BM_Simdjson_KeyAccess_Medium) ->Name("KeyAccess/Simdjson_DOM/Medium");
BENCHMARK(BM_Rapidjson_KeyAccess_Medium)->Name("KeyAccess/Rapidjson/Medium");

// ── Array Iteration ────────────────────────────────────────────────────
BENCHMARK(BM_Thoth_ArrayIteration_Array)    ->Name("ArrayIteration/Thoth/Array");
BENCHMARK(BM_Nlohmann_ArrayIteration_Array) ->Name("ArrayIteration/Nlohmann/Array");
BENCHMARK(BM_Simdjson_ArrayIteration_Array) ->Name("ArrayIteration/Simdjson_DOM/Array");
BENCHMARK(BM_Rapidjson_ArrayIteration_Array)->Name("ArrayIteration/Rapidjson/Array");

BENCHMARK(BM_Thoth_ArrayIteration_Large)    ->Name("ArrayIteration/Thoth/Large");
BENCHMARK(BM_Nlohmann_ArrayIteration_Large) ->Name("ArrayIteration/Nlohmann/Large");
BENCHMARK(BM_Simdjson_ArrayIteration_Large) ->Name("ArrayIteration/Simdjson_DOM/Large");
BENCHMARK(BM_Rapidjson_ArrayIteration_Large)->Name("ArrayIteration/Rapidjson/Large");

// ── Build Object ───────────────────────────────────────────────────────
BENCHMARK(BM_Thoth_BuildObject)    ->Name("Build/Object/Thoth");
BENCHMARK(BM_Nlohmann_BuildObject) ->Name("Build/Object/Nlohmann");
BENCHMARK(BM_Rapidjson_BuildObject)->Name("Build/Object/Rapidjson");

// ── Build Array ────────────────────────────────────────────────────────
BENCHMARK(BM_Thoth_BuildArray)    ->Name("Build/Array/Thoth")    ->RangeMultiplier(4)->Range(10, 1000);
BENCHMARK(BM_Nlohmann_BuildArray) ->Name("Build/Array/Nlohmann") ->RangeMultiplier(4)->Range(10, 1000);
BENCHMARK(BM_Rapidjson_BuildArray)->Name("Build/Array/Rapidjson")->RangeMultiplier(4)->Range(10, 1000);

// ── Type Checking ──────────────────────────────────────────────────────
BENCHMARK(BM_Thoth_TypeChecking)    ->Name("TypeChecking/Thoth/Medium");
BENCHMARK(BM_Nlohmann_TypeChecking) ->Name("TypeChecking/Nlohmann/Medium");
BENCHMARK(BM_Rapidjson_TypeChecking)->Name("TypeChecking/Rapidjson/Medium");

// ── Path Traversal ─────────────────────────────────────────────────────
BENCHMARK(BM_Thoth_PathTraversal_Nested)    ->Name("PathTraversal/Thoth/Nested");
BENCHMARK(BM_Nlohmann_PathTraversal_Nested) ->Name("PathTraversal/Nlohmann/Nested");
BENCHMARK(BM_Simdjson_PathTraversal_Nested) ->Name("PathTraversal/Simdjson_DOM/Nested");
BENCHMARK(BM_Rapidjson_PathTraversal_Nested)->Name("PathTraversal/Rapidjson/Nested");

// ── Round-trip ─────────────────────────────────────────────────────────
BENCHMARK(BM_Thoth_RoundTrip_Medium)    ->Name("RoundTrip/Thoth/Medium");
BENCHMARK(BM_Nlohmann_RoundTrip_Medium) ->Name("RoundTrip/Nlohmann/Medium");
BENCHMARK(BM_Rapidjson_RoundTrip_Medium)->Name("RoundTrip/Rapidjson/Medium");

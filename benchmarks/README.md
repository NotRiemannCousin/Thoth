# Thoth Benchmarks

Google Benchmark suites comparing **Thoth** against the most widely used C++ HTTP and JSON libraries.

## Libraries compared

| Domain | Thoth | Competitors |
|--------|-------|-------------|
| JSON   | `Thoth::NJson` | [nlohmann/json](https://github.com/nlohmann/json) · [simdjson](https://github.com/simdjson/simdjson) · [RapidJSON](https://github.com/Tencent/rapidjson) |
| HTTP   | `Thoth::Http::Client` | [libcurl](https://curl.se/libcurl/) · [cpp-httplib](https://github.com/yhirose/cpp-httplib) |

All HTTP competitors run in **synchronous / blocking** mode (matching `Client::Send<>()`).  
Thread-pool scenarios use `std::barrier` to synchronise all worker threads and start them simultaneously.

---

## Build

```bash
cmake -B build -DTHOTH_BUILD_BENCHMARKS=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(nproc)
```

### HTTP endpoint (optional)

By default the HTTP benchmarks target `httpbin.org:443`.  
For controlled/reproducible timing, point them at a local server:

```bash
cmake -B build -DTHOTH_BUILD_BENCHMARKS=ON \
      -DBENCH_HTTP_HOST=localhost \
      -DBENCH_HTTP_PORT=8080 \
      -DBENCH_HTTP_PATH=/ping \
      -DCMAKE_BUILD_TYPE=Release
```

A minimal echo server (Python) you can use locally:

```python
# server.py
from http.server import HTTPServer, BaseHTTPRequestHandler

class H(BaseHTTPRequestHandler):
    def do_GET(self):
        body = b'{"ok":true}'
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)
    def do_POST(self):
        self.do_GET()
    def log_message(self, *_): pass

HTTPServer(("localhost", 8080), H).serve_forever()
```

---

## Run

After building, binaries land in `build/bin/`:

```bash
# JSON benchmarks
./build/bin/ThothBench_Json --benchmark_out=results_json.json --benchmark_out_format=json

# HTTP benchmarks
./build/bin/ThothBench_Http --benchmark_out=results_http.json --benchmark_out_format=json

# Filter to a specific group
./build/bin/ThothBench_Json --benchmark_filter="Parse/.*Large"
./build/bin/ThothBench_Json --benchmark_filter="RoundTrip"

# Human-readable table
./build/bin/ThothBench_Json --benchmark_format=console --benchmark_counters_tabular=true
```

---

## JSON benchmark scenarios

| Scenario | Description |
|----------|-------------|
| `Parse/{lib}/{dataset}` | Full parse from `std::string` → DOM tree |
| `Parse/Thoth/{ds}/NoCopy` | Thoth zero-copy parse (`copyData=false`) — strings point into the caller's buffer |
| `Parse/Simdjson_DOM/{ds}` | simdjson DOM (fully materialised) |
| `Parse/Rapidjson/{ds}/InSitu` | RapidJSON in-situ parse (modifies buffer in-place) |
| `Stringify/{lib}/{dataset}` | DOM → string serialisation |
| `KeyAccess/{lib}/Medium` | Three top-level key look-ups on a parsed object |
| `ArrayIteration/{lib}/{dataset}` | Walk every element, read one string field |
| `Build/Object/{lib}` | Build a 7-field object with nested sub-object and array |
| `Build/Array/{lib}/N` | Build an N-element array of objects (N = 10…1000) |
| `TypeChecking/{lib}/Medium` | `isObject/isArray/isString/isNumber/isBool` on every user in medium.json |
| `PathTraversal/{lib}/Nested` | Drill 10 levels deep, read `meta.tag` |
| `RoundTrip/{lib}/Medium` | Parse → mutate one value → stringify |

### Datasets

| File | Size | Content |
|------|------|---------|
| `small.json` | ~150 B | Single flat object, 7 fields |
| `medium.json` | ~14 KB | 40 user objects with nested address |
| `large.json` | ~5 MB | 10 000-element array (from VsNlohmann example) |
| `nested.json` | ~15 KB | 20-level deep tree with siblings at each node |
| `numbers.json` | ~5 KB | 20×20 matrix of IEEE-754 floats |
| `array.json` | ~15 KB | 200-element flat-ish object array |
| `strings.json` | ~4 KB | Unicode, escaped chars, long strings |

---

## HTTP benchmark scenarios

| Scenario | Threads | Reqs / thread | Description |
|----------|---------|---------------|-------------|
| `Single_GET` | 1 | 1 | One blocking GET per iteration; measures TLS handshake + round-trip |
| `Sequential_GET/N` | 1 | N | N GETs in a loop; tests connection keep-alive reuse |
| `Reuse_GET/N` | 1 | N | Same, but with an explicit persistent handle where applicable |
| `Parallel_GET/T` | T | 1 | T threads each do one GET; stresses the connection pool |
| `ParallelReuse_GET/T` | T | 5 | T threads × 5 GETs; best-case throughput with pool contention |
| `BodyToString_GET` | 1 | 1 | GET + full response body accumulated into `std::string` |
| `POST_WithBody` | 1 | 1 | POST with a small JSON payload |

---

## Notes

- Build in **Release** (`-O2` / `/O2`) for meaningful results; Debug builds include assertions and extra safety checks that skew latency.
- Run on an idle machine, pinned to physical cores if possible (`taskset -c 0,2,4,6`).
- HTTP timings are dominated by network RTT when using `httpbin.org`. Use a local server for micro-comparisons.
- simdjson's **on-demand** API (`BM_Simdjson_Parse`) is intentionally lazy — most work happens during traversal, not `iterate()`. The **DOM** variant (`BM_Simdjson_DOM_Parse`) materialises the full tree eagerly and is the fair apples-to-apples comparison with Thoth/nlohmann/RapidJSON.
- Thoth's `ParseText(..., copyData=false)` keeps a `string_view` into the caller's buffer. It is faster but the `Json` tree becomes invalid if the source string is destroyed or modified.

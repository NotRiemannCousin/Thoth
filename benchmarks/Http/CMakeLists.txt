set(BENCH_HTTP_HOST "http://localhost" CACHE STRING "Host to benchmark HTTP")
set(BENCH_HTTP_PORT 443                CACHE STRING "Port to benchmark HTTP")
set(BENCH_HTTP_PATH "/dashboard/"      CACHE STRING "Path to benchmark HTTP")

add_executable(BenchHttp BenchHttp.cpp)

thoth_bench_target(BenchHttp)

target_compile_definitions(BenchHttp PRIVATE
        BENCH_HTTP_HOST=\"${BENCH_HTTP_HOST}\"
        BENCH_HTTP_PORT=${BENCH_HTTP_PORT}
        BENCH_HTTP_PATH=\"${BENCH_HTTP_PATH}\"
)

target_link_libraries(BenchHttp PRIVATE
        httplib::httplib
        OpenSSL::SSL
        OpenSSL::Crypto
        CURL::libcurl
)
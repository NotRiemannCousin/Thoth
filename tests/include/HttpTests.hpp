#pragma once

void QueryParamsTests();
void UrlTests();
void HeadersTests();
void RequestTests();

inline void HttpTests() {
    QueryParamsTests();
    UrlTests();
    HeadersTests();
    RequestTests();
}
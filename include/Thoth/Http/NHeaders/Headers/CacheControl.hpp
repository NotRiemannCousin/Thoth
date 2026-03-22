#pragma once
#include <variant>
#include <chrono>
#include <numeric>


// namespace Thoth::Http::NHeaders {
//     //! @brief RFC 9111 - HTTP Caching
//
//     namespace NCacheControl {
//         // Request
//         struct MaxAge{ std::chrono::seconds seconds; };
//         struct MaxStale{ std::chrono::seconds seconds{ (std::numeric_limits<uint32_t>::max)() }; };
//         struct MinFresh{ std::chrono::seconds seconds; };
//         struct NoCache{};
//         struct NoStore{};
//         struct NoTransform{};
//         struct OnlyIfCached{};
//
//         // Response
//         struct MustRevalidate {};
//         struct Public {};
//         struct Private {};
//         struct ProxyRevalidate {};
//         struct SMaxage { std::chrono::seconds seconds; };
//         // Also NoCache, NoStore, NoTransform and MaxAge.
//
//         // Extended
//
//
//     }
//
//     using CacheControl = std::variant<>;
// }

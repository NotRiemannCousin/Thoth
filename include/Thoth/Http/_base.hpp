#pragma once
#include <Thoth/Http/NHeaders/Headers.hpp>

namespace Thoth::Http {
    enum class VersionEnum : uint8_t { HTTP1_0, HTTP1_1, HTTP2, HTTP3, };

    //! @brief Exactly what you think it is.
    std::string_view VersionToString(VersionEnum version);


    template<class T>
    concept SizedReadableBodyConcept = std::ranges::input_range<T>
        && std::ranges::sized_range<T>
        && (
            std::same_as<std::ranges::range_value_t<T>, char> ||
            std::same_as<std::ranges::range_value_t<T>, unsigned char> ||
            std::same_as<std::ranges::range_value_t<T>, std::byte>
        );

    template<class T>
    concept ChunkedReadableBodyConcept = std::ranges::input_range<T>
        && SizedReadableBodyConcept<std::ranges::range_value_t<T>>;

    template<class T>
    concept ReadableBodyConcept = SizedReadableBodyConcept<T> || ChunkedReadableBodyConcept<T>;


    template<class T>
    concept WritableBodyConcept =
            std::ranges::output_range<T, char> ||
            std::ranges::output_range<T, unsigned char> ||
            std::ranges::output_range<T, std::byte>;



    template<class T>
    concept BodyConcept = ReadableBodyConcept<T> && WritableBodyConcept<T>;

    template<class F, class Body, class Head>
    concept BodyFactoryConcept = WritableBodyConcept<Body> &&
    requires (F f, const Head& head) {
        { std::invoke(f, head) } -> std::same_as<std::expected<Body, ExchangeError>>;
    };

    //! @brief Concept for any synchronous Hermes socket (Client or Server-side) over
    //! which Thoth's wire algorithms can operate. Hermes' ClientSocket and ServerSocket
    //! are already structurally identical on this transfer surface — this concept
    //! simply formalizes it on Thoth's side, so HTTP algorithms don't need to know
    //! whether they are talking to a client or server socket.
    //! @note The signature required here is purposely minimal (what SendBody actually
    //! uses today); it should be revisited when sending chunks (types beyond
    //! std::string_view) is tested.
    template<class S>
    concept ConnectionConcept = requires(S s, std::string_view data) {
        { s.socket };
        { s.Send(data) } -> std::same_as<Hermes::StreamByteOper>;
        { s.Close() } -> std::same_as<void>;
        { s.Abort() } -> std::same_as<void>;
    };


    namespace details_ {
        template<class Stream, class Head>
        struct ParseStage {
            Head data{};
            Stream stream;
        };

        template<class Stream, class Head, WritableBodyConcept Body>
        struct ParseCompleteStage : ParseStage<Stream, Head> {
            Body body{};
        };

        static constexpr std::string_view k_crlf     { "\r\n" };
        static constexpr std::string_view k_crlfCrlf { "\r\n\r\n" };
        static constexpr std::string_view k_lastChunk{ "0\r\n\r\n" };
    }
}

#include <Thoth/Http/_base.tpp>
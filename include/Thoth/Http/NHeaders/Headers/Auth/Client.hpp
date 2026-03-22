#pragma once
// Im not sure of how to do all of these things, so I'm freezing everything

// #include <chrono>
// #include <optional>
// #include <variant>
//
// #include <Thoth/Http/NHeaders/Auth/_base.hpp>
//
// namespace Thoth::Http::NHeaders::Client {
//     using SString = std::string;
//
//
//     //! @brief RFC 7617 — The 'Basic' HTTP Authentication Scheme
//     //!
//     //! Decodes to base64(username:password).
//     struct BasicPayload {
//         std::string username;
//         std::string password;
//     };
//
//     //! @brief RFC 6750 — The OAuth 2.0 Authorization Framework: Bearer Token Usage
//     //!
//     //! opaque bearer token.
//     struct BearerPayload {
//         SString token;
//     };
//
//     //! @brief RFC 7616 — Digest Access Authentication Payload
//     //!
//     //! key=value pairs.
//     struct DigestPayload {
//         SString uri;
//         SString response;
//
//         DigestAlgorithmEnum algorithm{ DigestAlgorithmEnum::Md5 };
//
//         struct ChallengeEcho {
//             SString realm;
//             SString nonce;
//             std::optional<SString> opaque;
//         } echo;
//
//         struct UserIdentity {
//             std::u8string username;
//             bool usesHash{};
//         } user;
//
//         struct QualityOfProtection {
//             bool usesIntegrity;
//             SString cnonce;
//             uint32_t nc;
//         };
//
//         std::optional<QualityOfProtection> protection;
//     };
//
//     //! @brief RFC 7486 — HTTP Origin-Bound Authentication (HOBA)
//     //!
//     //! Digital-signature-based origin-bound auth.
//     struct HobaPayload {
//         struct ChallengeEcho {
//             SString challenge;
//             SString nonce;
//             std::optional<SString> realm;
//         } echo;
//
//         struct KeyIdentity {
//             HobaKidTypeEnum type;
//             SString value;
//             std::optional<SString> pub;
//         };
//         std::optional<KeyIdentity> kid;
//
//         struct DeviceIdentity {
//             HobaDidTypeEnum type;
//             SString value;
//         };
//         std::optional<DeviceIdentity> did;
//
//         std::chrono::duration<uint32_t> maxAge{};
//         std::optional<SString> origin;
//         std::optional<uint8_t> result;
//
//         uint16_t alg{};
//         SString sig;
//     };
//
//     //! @brief RFC 8120 —  Mutual Authentication Protocol for HTTP
//     //!
//     //! key=value pairs.
//     struct MutualPayload {
//         std::vector<std::pair<SString, SString>> params;
//     };
//
//     //! @brief RFC 4559 —  SPNEGO-based Kerberos and NTLM HTTP Authentication in Microsoft Windows
//     struct NegotiatePayload {
//         SString token;
//     };
//
//     //! @brief RFC 8292 —  Voluntary Application Server Identification (VAPID) for Web Push
//     struct VapidPayload {
//         //! @brief JWT signed by the application server.
//         SString token;
//         //! @brief raw base64url-encoded public key.
//         SString key;
//     };
//
//     //! @brief RFC 7804 —  Salted Challenge Response HTTP Authentication Mechanism
//     struct ScramPayload {
//         SString message;
//     };
//
//     //! @brief AWS Signature Version 4
//     struct Aws4HmacSha256Payload {
//         SString algorithm;
//         SString accessKeyId;
//         SString credentialScope;
//         SString signedHeaders;
//         SString signature;
//     };
//
//
//     //! @brief Fallback for unknown/custom schemes
//     struct OtherPayload {
//         SString scheme;
//         SString payload;
//     };
//
//
//     //! @brief Class that deals with the auth-credentials.
//     struct AuthCredentials : std::variant<
//         BasicPayload,
//         BearerPayload,
//         DigestPayload,
//         HobaPayload,
//         MutualPayload,
//         NegotiatePayload,
//         VapidPayload,
//         ScramPayload,
//         Aws4HmacSha256Payload,
//         OtherPayload
//     > {
//     };
// }
//
// #include <Thoth/Http/NHeaders/Auth/Client.tpp>

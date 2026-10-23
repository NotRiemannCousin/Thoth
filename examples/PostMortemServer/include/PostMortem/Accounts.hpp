#pragma once

#include <chrono>
#include <expected>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

#include <Thoth/Http/Request/RequestHead.hpp>
#include <Thoth/Http/Response/StatusCodeEnum.hpp>

namespace PostMortem {
    //! @brief Simplified registered user.
    class Account {
    public:
        Account() = default;

        //! @brief Validates the pair and builds the account.
        //! @details 422 when the username is not letters/digits/`_`/`-` up to 24 bytes, or the password is empty
        //! or longer than 64 bytes. Plain validation, no strength rules, see the warning on @ref Accounts.
        static std::expected<Account, Thoth::ThothError> SignUp(std::string_view username, std::string_view password);

        [[nodiscard]] const std::string& Name() const noexcept;

        //! @brief Whether this password unlocks the account. Toy check: plain `==`, no hash, no salt.
        [[nodiscard]] bool Authenticate(std::string_view password) const noexcept;

    private:
        Account(std::string username, std::string password);

        std::string m_username{};
        std::string m_password{};
    };

    //! @brief One login: a random token bound to an account, valid for a fixed lifetime.
    class Session {
    public:
        Session() = default;

        Session(std::string token, std::string username, std::chrono::seconds lifetime);

        [[nodiscard]] const std::string& Token() const noexcept;
        //! @brief The account this session belongs to. The Accounts manager pairs it back into an Account.
        [[nodiscard]] const std::string& Username() const noexcept;

        [[nodiscard]] bool Expired() const noexcept;

    private:
        std::string m_token{};
        std::string m_username{};
        std::chrono::system_clock::time_point m_expiresAt{};
    };

    //! @brief The account system: sign up, log in, log out, and "who is this request?".
    //!
    //! @warning **Do not copy this into a real application.** Thoth is a networking library, it ships nothing for
    //! authentication, so this example fakes it in the simplest layered way possible:
    //!  - an unknown username silently creates the account (log in and sign up are the same form);
    //!  - passwords are kept in plain text, in memory, and compared with `==` (no hashing, no salt, no rate limit);
    //!  - sessions are random tokens in a `HttpOnly; SameSite=Lax` cookie that live for @ref kSessionLifetime,
    //!    there is no CSRF token and no HTTPS (so no `Secure` flag).
    //! A real service should use a vetted library or an identity provider for all of this.
    //!
    //! The state is a singleton (like Board): a real deployment would put it behind another service or a
    //! database, the example keeps it in process so the routes stay about the HTTP.
    struct Accounts {
        //! @brief The cookie the session token travels in.
        static constexpr std::string_view kSessionCookie{ "session" };
        //! @brief How long a session lasts. Fixed, no sliding renewal.
        static constexpr std::chrono::hours kSessionLifetime{ 24 };

        static Accounts& Instance();

        //! @brief Logs in, creating the account on the first use.
        //! @details 422 when @ref Account::SignUp rejects the pair, 401 when the password is wrong. Returns the
        //! fresh @ref Session; its token is what goes into the cookie (see the routes in main.cpp).
        std::expected<Session, Thoth::ThothError> LogIn(std::string_view username, std::string_view password);

        //! @brief Ends a session. Unknown or expired tokens are ignored.
        void LogOut(std::string_view token);

        //! @brief The account behind the @ref kSessionCookie cookie of this request head. `nullopt` is a visitor
        //! (no cookie, unknown token or expired session).
        std::optional<Account> WhoAmI(const Thoth::Http::RequestHead& head) const;

    private:
        Accounts() = default;

        mutable std::mutex m_mutex{};
        std::map<std::string, Account, std::less<>> m_accounts{};  //!< username -> account (plain text passwords!)
        std::map<std::string, Session, std::less<>> m_sessions{};  //!< token -> session
    };

}

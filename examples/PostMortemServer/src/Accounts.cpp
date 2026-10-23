#include <PostMortem/Accounts.hpp>

#include <algorithm>
#include <format>
#include <random>
#include <utility>

namespace {
    constexpr std::size_t kMaxUsernameBytes{ 24 };
    constexpr std::size_t kMaxPasswordBytes{ 64 };

    bool ValidUsername(std::string_view name) {
        return !name.empty() && name.size() <= kMaxUsernameBytes &&
            name.find_first_not_of(Thoth::String::CharSequences::k_base64Url) == std::string::npos;
    }

    std::string NewToken() {
        std::random_device device{};
        return { std::format("{:08x}{:08x}{:08x}{:08x}", device(), device(), device(), device()) };
    }
}

namespace PostMortem {
    std::expected<Account, Thoth::ThothError> Account::SignUp(std::string_view username, std::string_view password) {
        if (!ValidUsername(username) || password.empty() || password.size() > kMaxPasswordBytes)
            return std::unexpected{ Thoth::Http::StatusCodeEnum::UnprocessableContent };

        return Account{ std::string{ username }, std::string{ password } };
    }

    const std::string& Account::Name() const noexcept {
        return m_username;
    }

    Account::Account(std::string username, std::string password)
        : m_username{ std::move(username) }, m_password{ std::move(password) } {}

    bool Account::Authenticate(std::string_view password) const noexcept {
        return m_password == password; // eMudhra security level
    }

    Session::Session(std::string token, std::string username, std::chrono::seconds lifetime)
        : m_token{ std::move(token) },
          m_username{ std::move(username) },
          m_expiresAt{ std::chrono::system_clock::now() + lifetime } {}

    const std::string& Session::Token() const noexcept {
        return m_token;
    }

    const std::string& Session::Username() const noexcept {
        return m_username;
    }

    bool Session::Expired() const noexcept {
        return std::chrono::system_clock::now() >= m_expiresAt;
    }

    Accounts& Accounts::Instance() {
        static Accounts self{};
        return self;
    }

    std::expected<Session, Thoth::ThothError> Accounts::LogIn(std::string_view username, std::string_view password) {
        const auto validateLogIn{ [&](Account candidate) -> std::expected<Session, Thoth::ThothError> {
            const std::scoped_lock lock{ m_mutex };

            // Expired sessions leave the map when somebody logs in.
            std::erase_if(m_sessions, [](const auto& entry) { return entry.second.Expired(); });

            const auto account{ m_accounts.find(username) };
            if (account == m_accounts.end())
                m_accounts.emplace(std::string{ username }, std::move(candidate));
            else if (!account->second.Authenticate(password))
                return std::unexpected{ Thoth::Http::StatusCodeEnum::Unauthorized };

            const Session session{ NewToken(), std::string{ username }, kSessionLifetime };
            const auto [it, _]{ m_sessions.emplace(session.Token(), session) };
            return it->second;
        } };

        return Account::SignUp(username, password)
                .and_then(validateLogIn);

    }

    void Accounts::LogOut(std::string_view token) {
        const std::scoped_lock lock{ m_mutex };

        m_sessions.erase(std::string{ token });
    }

    std::optional<Account> Accounts::WhoAmI(const Thoth::Http::RequestHead& head) const {
        std::optional<std::string> token{};
        for (const auto& [name, value] : head.headers.GetCookiesView())
            if (name == kSessionCookie)
                token = value;

        if (!token)
            return std::nullopt;

        const std::scoped_lock lock{ m_mutex };

        const auto session{ m_sessions.find(*token) };
        if (session == m_sessions.end() || session->second.Expired())
            return std::nullopt; // expired sessions leave the map on the next LogIn sweep

        const auto account{ m_accounts.find(session->second.Username()) };
        if (account == m_accounts.end())
            return std::nullopt;

        return account->second;
    }

}

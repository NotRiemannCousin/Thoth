#pragma once

#include <chrono>
#include <cstdint>
#include <expected>
#include <mutex>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <Thoth/ThothError.hpp>

namespace PostMortem {
    //! @brief An answer to a post or to another reply. It always lives inside the thread of a post.
    struct Reply {
        std::uint64_t            id{};
        std::string              author{};
        //! @brief Id of whatever this reply quotes (`>>#0007`), a post or another reply of the same thread.
        std::uint64_t            target{};
        std::string              content{};
        std::chrono::sys_seconds createdAt{};
    };

    //! @brief A thread starter.
    struct Post {
        std::uint64_t            id{};
        std::string              author{};
        std::string              title{};
        std::vector<std::string> tags{};
        std::string              content{};
        std::chrono::sys_seconds createdAt{};
        std::vector<Reply>       replies{};
    };

    //! @brief The whole forum, kept in memory (restart the server and it is gone).
    //!
    //! The server runs handlers on several worker threads at once (FastIoLoop), so every access goes through a mutex.
    //! Reads hand back copies, a request never holds the lock while it renders a page. Posts and replies share the same
    //! id counter, that way `>>#0007` is unambiguous no matter what it points to.
    //!
    //! @note Who is allowed to change something is decided here (403), the caller identity is decided by @ref Accounts
    //! (see @ref WhoAmI).
    struct Board {
        static Board& Instance();

        //! @brief Stores a new post and returns its id. Validation is the caller's job.
        std::uint64_t AddPost(std::string author, std::string title, std::vector<std::string> tags, std::string content);

        //! @brief Answers `target` (a post or a reply). Fails with 404 when the target doesn't exist.
        std::expected<std::uint64_t, Thoth::ThothError> AddReply(std::string author, std::uint64_t target, std::string content);

        //! @brief Replaces title and content of a post, only its author can do it (403). Returns the updated post.
        std::expected<Post, Thoth::ThothError> EditPost(
            std::uint64_t id, std::string_view user, std::string title, std::string content);

        //! @brief Removes a post and its whole thread, only its author can do it (403).
        std::expected<void, Thoth::ThothError> DeletePost(std::uint64_t id, std::string_view user);

        //! @brief Removes one reply, only its author can do it (403). Replies quoting it keep the dangling `>>#id`.
        std::expected<void, Thoth::ThothError> DeleteReply(std::uint64_t id, std::string_view user);

        //! @brief Whether `id` is a post or a reply.
        [[nodiscard]] bool Exists(std::uint64_t id) const;

        //! @brief Every post, newest first. A non-empty `tags` keeps only the posts carrying *every* selected tag.
        [[nodiscard]] std::vector<Post> Posts(std::span<std::string> tags = {}) const;

        //! @brief Every tag in use, sorted, without repeats.
        [[nodiscard]] std::vector<std::string> Tags() const;

    private:
        Board() = default;
        Board(const Board&)            = delete;
        Board& operator=(const Board&) = delete;

        mutable std::mutex m_mutex{};
        std::vector<Post>  m_posts{};
        std::uint64_t      m_nextId{ 1 };
    };

}

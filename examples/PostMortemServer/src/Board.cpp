#include <PostMortem/Board.hpp>

#include <algorithm>
#include <ranges>
#include <set>
#include <utility>

namespace PostMortem {

    namespace {
        std::chrono::sys_seconds Now() {
            return std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
        }

        bool HasReply(const Post& post, const std::uint64_t id) {
            return std::ranges::contains(post.replies, id, &Reply::id);
        }

        auto IsPostOrReply(std::uint64_t id) {
            return [id](const Post& post) { return post.id == id || HasReply(post, id); };
        }

        auto ContainTags(std::span<std::string> tags) {
            return [tags](const Post& post) {
                return std::ranges::all_of(tags, [&](const auto& elem) {
                    return std::ranges::contains(post.tags, elem);
                });
            };
        }
    }

    std::uint64_t Board::AddPost(
        std::string author, std::string title, std::vector<std::string> tags, std::string content
    ) {
        const std::scoped_lock lock{ m_mutex };

        const std::uint64_t id{ m_nextId++ };
        m_posts.push_back(Post{
            .id        = id,
            .author    = std::move(author),
            .title     = std::move(title),
            .tags      = std::move(tags),
            .content   = std::move(content),
            .createdAt = Now(),
        });
        return id;
    }

    std::expected<std::uint64_t, Thoth::ThothError> Board::AddReply(
            std::string author, std::uint64_t target, std::string content) {
        const std::scoped_lock lock{ m_mutex };

        // Find the thread the target belongs to: either the post itself or one of its replies.
        const auto thread{ std::ranges::find_if(m_posts, IsPostOrReply(target)) };

        if (thread == m_posts.end())
            return std::unexpected{ Thoth::Http::StatusCodeEnum::NotFound };

        const std::uint64_t id{ m_nextId++ };
        thread->replies.push_back(Reply{
            .id        = id,
            .author    = std::move(author),
            .target    = target,
            .content   = std::move(content),
            .createdAt = Now(),
        });
        return id;
    }

    std::expected<Post, Thoth::ThothError> Board::EditPost(
        const std::uint64_t id, const std::string_view user, std::string title, std::string content
    ) {
        const std::scoped_lock lock{ m_mutex };

        const auto post{ std::ranges::find(m_posts, id, &Post::id) };
        if (post == m_posts.end())
            return std::unexpected{ Thoth::Http::StatusCodeEnum::NotFound };
        if (post->author != user)
            return std::unexpected{ Thoth::Http::StatusCodeEnum::Forbidden };

        post->title   = std::move(title);
        post->content = std::move(content);
        return *post;
    }

    std::expected<void, Thoth::ThothError> Board::DeletePost(std::uint64_t id, std::string_view user) {
        const std::scoped_lock lock{ m_mutex };

        const auto post{ std::ranges::find(m_posts, id, &Post::id) };
        if (post == m_posts.end())
            return std::unexpected{ Thoth::Http::StatusCodeEnum::NotFound };
        if (post->author != user)
            return std::unexpected{ Thoth::Http::StatusCodeEnum::Forbidden };

        m_posts.erase(post);
        return {};
    }

    std::expected<void, Thoth::ThothError> Board::DeleteReply(std::uint64_t id, std::string_view user) {
        const std::scoped_lock lock{ m_mutex };

        for (Post& post : m_posts) {
            const auto reply{ std::ranges::find(post.replies, id, &Reply::id) };
            if (reply == post.replies.end())
                continue;
            if (reply->author != user)
                return std::unexpected{ Thoth::Http::StatusCodeEnum::Forbidden };

            post.replies.erase(reply);
            return {};
        }

        return std::unexpected{ Thoth::Http::StatusCodeEnum::NotFound };
    }

    bool Board::Exists(const std::uint64_t id) const {
        const std::scoped_lock lock{ m_mutex };

        return std::ranges::any_of(m_posts, IsPostOrReply(id));
    }

    std::vector<Post> Board::Posts(const std::span<std::string> tags) const {
        const std::scoped_lock lock{ m_mutex };

        return { std::from_range,
            m_posts | std::views::reverse | std::views::filter(ContainTags(tags)) };
    }

    std::vector<std::string> Board::Tags() const {
        const std::scoped_lock lock{ m_mutex };

        std::set unique{ std::from_range,
            m_posts | std::views::transform(&Post::tags) | std::views::join };

        return { std::from_range, std::move(unique) };
    }

    Board& Board::Instance() {
        static Board instance{};
        return instance;
    }
}

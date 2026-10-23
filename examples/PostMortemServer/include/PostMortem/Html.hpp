#pragma once

#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <unordered_map>

namespace PostMortem::Html {

    //! @brief Opts one argument out of the automatic escaping of @ref Component::Render.
    //! @details Every component argument that reads as text (`std::string`, `std::string_view`, `const char*`...) is
    //! HTML-escaped before it reaches the template. Use `Raw` for blocks that are already HTML, like a component
    //! rendered inside another one.
    struct Raw { std::string_view text{}; };

    //! @brief The singleton cache behind every @ref Component (the files of the public dir, read once, on demand).
    //! @details The files are preprocessed into format strings (`%% ... %%` toggles the format regions) so you can use
    //! change the template but keep the data.
    //!
    //! Different from a real deployment, here everything is local so the cache is a singleton and the routes never pass
    //! it around.
    struct Components {
        static Components& Instance();

        //! @brief Sets the public dir.
        //! @throws std::runtime_error when the dir does not exist.
        void SetRoot(std::filesystem::path root);

        //! @brief The content of `root / path`, loaded on the first use.
        //! @throws std::runtime_error when the file cannot be read (a request then fails instead of the server).
        [[nodiscard]] std::shared_ptr<const std::string> Get(const std::filesystem::path& path);

        //! @brief Discards every cached file. They will be re-cached in the next request.
        void Reload();

        //! @brief Get the root path.
        [[nodiscard]] const std::filesystem::path& Root() const noexcept;

    private:
        Components() = default;

        std::filesystem::path m_root{};
        std::unordered_map<std::string, std::shared_ptr<const std::string>> m_files{};
        mutable std::mutex m_mutex{};
    };

    //! @brief Escapes user-provided text for HTML (`&`, `<`, `>`, `"`, `'`).
    [[nodiscard]] std::string EscapeHtml(std::string_view raw);

    //! @brief A `*.comp.html` file under the public dir, formatted per request.
    //! @details A Component holds only the file path, the bytes live in the @ref Components cache. The file is a
    //! `std::format` string: everything inside `%% ... %%` is parsed as a format syntax, the rest of the file is
    //! literal (braces there need no escaping).
    //!
    //! @note std::format are positional (`{0}`, `{1}`...) so one argument can fill several spots of the same file.
    //!
    //! Render() escapes string arguments automatically (and passes numbers, dates and @ref Raw through), so the
    //! call sites carry user text without a single `EscapeHtml`.
    //!
    //! @code{.cpp}
    //! const Html::Component reply{ "components/reply.comp.html" };
    //! reply.Render(reply.id, reply.author, reply.createdAt, Html::Raw{ actions }, reply.target, reply.content);
    //! @endcode
    class Component {
    public:
        //! @param path File path relative to the public dir, e.g. `"components/post.comp.html"`.
        explicit Component(std::filesystem::path path) noexcept;

        //! @brief Formats the component file with the given arguments.
        //! @throws std::runtime_error when the file cannot be read, std::format_error when the placeholders and
        //! the arguments disagree (the path of the component is part of the message).
        template<class... Args>
        [[nodiscard]] std::string Render(Args&&... args) const;

    private:
        //! @brief Escapes text arguments, lets everything else (numbers, time points, @ref Raw) through.
        template<class T>
        static decltype(auto) Prepare(T&& arg);

        std::filesystem::path m_path{};
    };
}

#include <PostMortem/Html.tpp>
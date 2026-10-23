#include <PostMortem/Html.hpp>

#include <fstream>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <utility>


namespace {
    // Escapes '{' -> '{{' and '}' -> '}}' outside '%% ... %%' toggle blocks.
    // Inside `%% ... %%` the file is a std::format string, outside them it is literal HTML, so a stray brace in the
    // markup can never crash a format call.
    std::shared_ptr<const std::string> LoadAndParse(const std::filesystem::path& file) {
        const std::ifstream stream{ file, std::ios::binary };
        if (!stream.is_open())
            throw std::runtime_error{ "cannot open " + file.string() };

        std::ostringstream buffer{};
        buffer << stream.rdbuf();

        const std::string data{ buffer.str() };
        std::string_view raw{ data };

        std::string processed{};
        processed.reserve(raw.size() + raw.size() / 8);

        bool escapeBraces{ true };

        while (!raw.empty()) {
            if (raw.starts_with("%%")) {
                escapeBraces = !escapeBraces;
                raw.remove_prefix(2);
                continue;
            }
            const char c{ raw.front() };
            raw.remove_prefix(1);

            if (!escapeBraces) processed += c;
            else if (c == '{') processed += "{{";
            else if (c == '}') processed += "}}";
            else               processed += c;
        }

        if (!escapeBraces)
            throw std::runtime_error{ "unclosed '%%' in " + file.string() };

        return std::make_shared<const std::string>(std::move(processed));
    }
}

namespace PostMortem::Html {
    Components& Components::Instance() {
        static Components self{};
        return self;
    }

    void Components::SetRoot(std::filesystem::path root) {
        if (!std::filesystem::is_directory(root))
            throw std::runtime_error{ "public dir not found: " + root.string() };

        const std::scoped_lock lock{ m_mutex };
        m_root = std::move(root);
        m_files.clear();
    }

    std::shared_ptr<const std::string> Components::Get(const std::filesystem::path& path) {
        const std::string key{ path.string() };

        {
            const std::scoped_lock lock{ m_mutex };
            if (const auto file{ m_files.find(key) }; file != m_files.end())
                return file->second;
        }

        auto loaded{ LoadAndParse(m_root / path) };
        const std::scoped_lock lock{ m_mutex };
        return m_files.emplace(key, std::move(loaded)).first->second;
    }

    void Components::Reload() {
        const std::scoped_lock lock{ m_mutex };
        m_files.clear();
    }

    const std::filesystem::path& Components::Root() const noexcept {
        return m_root;
    }

    std::string EscapeHtml(const std::string_view raw) {
        std::string escaped{};
        escaped.reserve(raw.size());

        for (const char c : raw) {
            switch (c) {
                case '&':  escaped += "&amp;";  break;
                case '<':  escaped += "&lt;";   break;
                case '>':  escaped += "&gt;";   break;
                case '"':  escaped += "&quot;"; break;
                case '\'': escaped += "&#39;";  break;
                default:   escaped += c;        break;
            }
        }

        return escaped;
    }

    Component::Component(std::filesystem::path path) noexcept
            : m_path{ std::move(path) } {}
}

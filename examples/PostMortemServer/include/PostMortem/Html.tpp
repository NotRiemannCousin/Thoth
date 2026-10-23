#pragma once

namespace PostMortem::Html {
    template<class ... Args>
    std::string Component::Render(Args&&... args) const {
        // The shared_ptr keeps the bytes alive even if another thread Reloads the cache mid-format.
        const auto source{ Components::Instance().Get(m_path) };

        // Prepared values live in a tuple, so std::vformat reads lvalues that outlive the call.
        const auto prepared{ std::tuple{ Prepare(std::forward<Args>(args))... } };

        try {
            return std::apply([&source](const auto&... values) {
                return std::vformat(*source, std::make_format_args(values...));
            }, prepared);
        } catch (const std::format_error& error) {
            throw std::format_error{ std::format("{}: {}", m_path.string(), error.what()) };
        }
    }

    template<class T>
    decltype(auto) Component::Prepare(T&& arg) {
        if constexpr (std::same_as<std::remove_cvref_t<T>, Raw>) {
            return std::string_view{ arg.text };
        } else if constexpr (std::convertible_to<T, std::string_view>) {
            return EscapeHtml(std::string_view{ arg });
        } else {
            return std::forward<T>(arg);
        }
    }
}
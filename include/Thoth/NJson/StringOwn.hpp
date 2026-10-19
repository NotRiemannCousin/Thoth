#pragma once
#include <memory>
#include <string>
#include <string_view>

namespace Thoth::NJson {
    struct StringRef;

    //! @brief Managed owned string wrapper designed to fit in 8 bytes (pointer size) with deep-copy semantics required
    //! by Copy-On-Write structures.
    struct StringOwn {
        StringOwn() noexcept = default;
        StringOwn(std::string str);
        StringOwn(std::string_view str);
        StringOwn(const char* str);

        StringOwn(const StringRef& ref);

        StringOwn(const StringOwn& other);
        StringOwn(StringOwn&& other) noexcept = default;

        StringOwn& operator=(const StringOwn& other);
        StringOwn& operator=(StringOwn&& other) noexcept = default;
        StringOwn& operator=(std::string str);
        StringOwn& operator=(std::string_view str);

        operator std::string_view() const noexcept;
        operator std::string() const noexcept;

        [[nodiscard]] std::string_view view() const noexcept;
        [[nodiscard]] const std::string& string() const noexcept;
        [[nodiscard]] std::string& string() noexcept;

        bool operator==(const StringOwn& other) const noexcept;
        bool operator==(std::string_view other) const noexcept;

    private:
        std::unique_ptr<std::string> m_data;
    };
}

#include <Thoth/NJson/StringOwn.tpp>
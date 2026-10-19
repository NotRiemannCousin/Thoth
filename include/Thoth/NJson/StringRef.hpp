#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace Thoth::NJson {

    struct StringRef {
        explicit StringRef(const std::string& str);

        StringRef() noexcept = default;
        StringRef(StringRef&&) noexcept = default;
        StringRef(const StringRef&) noexcept = default;
        StringRef& operator=(StringRef&&) noexcept = default;
        StringRef& operator=(const StringRef&) noexcept = default;


        StringRef(std::string_view other, const std::shared_ptr<const char[]>& base) noexcept;

        [[nodiscard]] static StringRef FromOwned(std::string&& str);

        operator std::string_view() const noexcept;
        operator std::string() const;

        std::string_view AsStringView() const noexcept;

        bool operator==(const StringRef& other) const noexcept;

    private:
        std::uint32_t m_offset{};
        std::uint32_t m_length{};
        std::shared_ptr<const char[]> m_data;
    };

}

#include <Thoth/NJson/StringRef.tpp>
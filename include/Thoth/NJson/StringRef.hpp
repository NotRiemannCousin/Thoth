#pragma once
#include <memory>
#include <string_view>
#include <string>

namespace Thoth::NJson {
    struct StringRef {
        std::string_view str;

        StringRef() noexcept = default;
        StringRef(StringRef&&) noexcept = default;
        StringRef(const StringRef&) = default;
        StringRef(std::string_view str, std::shared_ptr<std::string> _data);

        // NOLINTNEXTLINE(*)
        operator std::string_view() const noexcept;


        StringRef& operator=(StringRef&&) noexcept = default;
        StringRef& operator=(const StringRef&) = default;

        bool operator==(const StringRef&) const = default;

    private:
        std::shared_ptr<std::string> _data;
        // it will keep the buffer alive despite everything.
        // TODO: FUTURE: `std::shared_ptr<std::string>` causes double allocation, change it later.
    };
}

#include <Thoth/NJson/StringRef.tpp>
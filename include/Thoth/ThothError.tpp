#pragma once
#include <ranges>

namespace Thoth {

    constexpr auto ThothError::FromError() {
        return []<typename T>(T&& error) {
            return ThothError{ std::forward<T>(error) };
        };
    }

    template<class T>
    constexpr bool ThothError::Is() const {
        return std::holds_alternative<T>(static_cast<const ThothErrorBase&>(*this));
    }

    template <class T>
    constexpr T ThothError::As() const {
        return std::get<T>(static_cast<const ThothErrorBase&>(*this));
    }

    template <class T>
    std::optional<T> ThothError::Ensure() const {
        const auto* value{ std::get_if<T>(static_cast<const ThothErrorBase*>(this)) };

        if (value != nullptr)
            return *value;

        return std::nullopt;
    }


    template<class T>
    constexpr bool ThothError::operator==(const T& rhs) const  {
        const T* ptr{ std::get_if<T>(this) };
        return ptr != nullptr && *ptr == rhs;
    }
}

template<>
struct std::formatter<Thoth::GenericError> {
    constexpr auto parse(auto& ctx) {
        return ctx.begin();
    }

    auto format(const Thoth::GenericError err, auto& ctx) const {
        return std::format_to(ctx.out(), "{}", err.error);
    }
};

template<>
struct std::formatter<Thoth::ThothError> {
    using ThothError = Thoth::ThothError;

    using JsonParseError        = Thoth::NJson::JsonParseError;
    using JsonGetError          = Thoth::NJson::JsonGetError;
    using JsonFindError         = Thoth::NJson::JsonFindError;
    using JsonSearchError       = Thoth::NJson::JsonSearchError;
    using GenericError          = Thoth::GenericError;

    constexpr auto parse(auto& ctx) {
        return ctx.begin();
    }

    auto format(const ThothError &error, auto& ctx) const {
        namespace rg = std::ranges;
        namespace vs = std::views;

        return std::visit([&ctx](const auto& err) mutable {
            return std::format_to(ctx.out(), "{}", err);
        }, error);
    }
};
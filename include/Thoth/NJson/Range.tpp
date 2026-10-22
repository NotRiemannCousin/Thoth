#pragma once

namespace Thoth::NJson {

#pragma region JsonIterator

    template<bool IsConst, bool IsItems>
    constexpr JsonIterator<IsConst, IsItems>::JsonIterator(ObjectIter it) : m_state{ it } {}

    template<bool IsConst, bool IsItems>
    constexpr JsonIterator<IsConst, IsItems>::JsonIterator(ArrayIter it, std::size_t index)
        : m_state{ std::in_place_type<ArrayProxyIter>, index, it } {}

    template<bool IsConst, bool IsItems>
    template<bool WasConst>
        requires IsConst && (!WasConst)
    JsonIterator<IsConst, IsItems>::JsonIterator(const JsonIterator<WasConst, IsItems>& other) {
        m_state = std::visit([](const auto& s) -> State {
            return State{ s };
        }, other.m_state);
    }

    template<bool IsConst, bool IsItems>
    auto JsonIterator<IsConst, IsItems>::operator*() const -> reference {
        return std::visit([](const auto& s) -> reference {
            using S_t = std::remove_cvref_t<decltype(s)>;
            if constexpr (std::same_as<S_t, ObjectIter>) {
                if constexpr (IsItems) return { KeyView{ s->first }, s->second };
                else                   return s->second;
            } else {
                if constexpr (IsItems) return { KeyView{ static_cast<int>(s.first) }, *s.second };
                else                   return *s.second;
            }
        }, m_state);
    }

    template<bool IsConst, bool IsItems>
    JsonIterator<IsConst, IsItems>& JsonIterator<IsConst, IsItems>::operator++() {
        std::visit([](auto& s) {
            using S_t = std::remove_cvref_t<decltype(s)>;
            if constexpr (std::same_as<S_t, ObjectIter>) {
                ++s;
            } else {
                ++s.first;
                ++s.second;
            }
        }, m_state);
        return *this;
    }

    template<bool IsConst, bool IsItems>
    JsonIterator<IsConst, IsItems> JsonIterator<IsConst, IsItems>::operator++(int) {
        const auto tmp{ *this };
        ++(*this);
        return tmp;
    }

    template<bool IsConst, bool IsItems>
    JsonIterator<IsConst, IsItems>& JsonIterator<IsConst, IsItems>::operator--() {
        std::visit([](auto& s) {
            using S_t = std::remove_cvref_t<decltype(s)>;
            if constexpr (std::same_as<S_t, ObjectIter>) {
                --s;
            } else {
                --s.first;
                --s.second;
            }
        }, m_state);
        return *this;
    }

    template<bool IsConst, bool IsItems>
    JsonIterator<IsConst, IsItems> JsonIterator<IsConst, IsItems>::operator--(int) {
        const auto tmp{ *this };
        --(*this);
        return tmp;
    }

#pragma endregion

#pragma region JsonItemsView

    template<bool IsConst>
    constexpr JsonItemsView<IsConst>::JsonItemsView(JsonPtr parent) noexcept : m_parent{ parent } {}

    template<bool IsConst>
    template<bool WasConst>
        requires IsConst && (!WasConst)
    JsonItemsView<IsConst>::JsonItemsView(const JsonItemsView<WasConst>& other) noexcept : m_parent{ other.m_parent } {}

    template<bool IsConst>
    template<class>
    auto JsonItemsView<IsConst>::begin() noexcept -> iterator requires (!IsConst) {
        if (!m_parent) return {};

        if (auto obj{ m_parent->template Ensure<Object>() })
            return iterator{ (**obj)->begin() };
        if (auto arr{ m_parent->template Ensure<Array>() })
            return iterator{ (*arr)->begin(), 0 };

        return {};
    }

    template<bool IsConst>
    template<class>
    auto JsonItemsView<IsConst>::end() noexcept -> iterator requires (!IsConst) {
        if (!m_parent) return {};

        if (auto obj{ m_parent->template Ensure<Object>() })
            return iterator{ (**obj)->end() };
        if (auto arr{ m_parent->template Ensure<Array>() })
            return iterator{ (*arr)->end(), (*arr)->size() };

        return {};
    }

    template<bool IsConst>
    auto JsonItemsView<IsConst>::begin() const noexcept -> const_iterator {
        if (!m_parent) return {};

        if (auto obj{ m_parent->template Ensure<Object>() })
            return const_iterator{ (**obj)->begin() };
        if (auto arr{ m_parent->template Ensure<Array>() })
            return const_iterator{ (*arr)->begin(), 0 };

        return {};
    }

    template<bool IsConst>
    auto JsonItemsView<IsConst>::end() const noexcept -> const_iterator {
        if (!m_parent) return {};

        if (auto obj{ m_parent->template Ensure<Object>() })
            return iterator{ (**obj)->end() };
        if (auto arr{ m_parent->template Ensure<Array>() })
            return iterator{ (*arr)->end(), (*arr)->size() };

        return {};
    }

    template<bool IsConst>
    bool JsonItemsView<IsConst>::empty() const {
        return m_parent == nullptr || m_parent->Empty();
    }

    template<bool IsConst>
    std::size_t JsonItemsView<IsConst>::size() const {
        return m_parent == nullptr ? std::size_t{ 0 } : m_parent->Size();
    }

#pragma endregion

}

template<bool IsConst>
inline constexpr bool std::ranges::enable_borrowed_range<Thoth::NJson::JsonItemsView<IsConst>> = true;

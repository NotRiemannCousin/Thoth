// Implementation of JsonIterator / JsonItemsView (see Range.hpp)

namespace Thoth::NJson {

#pragma region JsonIterator

    template<bool IsConst, bool IsItems>
    JsonIterator<IsConst, IsItems>::JsonIterator(JsonPtr parent, const bool atEnd) {
        if (parent == nullptr)
            return;

        if (parent->template IsOf<Array>()) {
            // ArrayState tracks both the iterator and the logical index (needed
            // for IsItems=true, ignored otherwise). As<Array>() returns a const ref
            // when parent is const Json*, via deducing-this.
            auto& arr{ parent->template As<Array>() };
            m_state = atEnd
                ? ArrayState{ arr.end(), static_cast<std::ptrdiff_t>(arr.size()) }
                : ArrayState{ arr.begin(), std::ptrdiff_t{ 0 } };
        }
        else if (parent->template IsOf<Object>()) {
            auto& obj{ *parent->template As<Object>() };
            m_state = atEnd
                ? ObjectState{ std::ranges::end(obj) }
                : ObjectState{ std::ranges::begin(obj) };
        }
        // else: scalar → monostate (empty range)
    }

    template<bool IsConst, bool IsItems>
    template<bool WasConst>
        requires IsConst && (!WasConst)
    JsonIterator<IsConst, IsItems>::JsonIterator(const JsonIterator<WasConst, IsItems>& other) {
        // Converting from mutable to const changes the iterator variant member
        // types, so we rebuild via visit.
        m_state = std::visit([](const auto& s) -> State {
            using S = std::remove_cvref_t<decltype(s)>;
            if constexpr (std::same_as<S, Scalar>)
                return Scalar{};
            else if constexpr (std::same_as<S, ObjectState>)
                return ObjectState{ s.iter };
            else
                return ArrayState{ s.iter, s.index };
        }, other.m_state);
    }

    template<bool IsConst, bool IsItems>
    auto JsonIterator<IsConst, IsItems>::operator*() const -> reference {
        assert(!std::holds_alternative<Scalar>(m_state) && "Dereferencing end() of empty range");

        return std::visit([]<class S>(const S& s) -> reference {
            using S_t = std::remove_cvref_t<S>;

            if constexpr (std::same_as<S_t, ArrayState>) {
                if constexpr (IsItems)
                    return { KeyView{ static_cast<int>(s.index) }, s.iter[0] };
                else
                    return s.iter[0];
            }
            else {  // ObjectState
                if constexpr (IsItems)
                    return { KeyView{ s.iter->first }, s.iter->second };
                else
                    return s.iter->second;
            }
        }, m_state);
    }

    template<bool IsConst, bool IsItems>
    auto JsonIterator<IsConst, IsItems>::operator->() const -> JsonPtr
        requires (!IsItems)
    {
        return &**this;
    }

    template<bool IsConst, bool IsItems>
    JsonIterator<IsConst, IsItems>& JsonIterator<IsConst, IsItems>::operator++() {
        std::visit([]<class S>(S& s) {
            using S_t = std::remove_cvref_t<S>;
            if constexpr (std::same_as<S_t, ArrayState>) {
                ++s.iter; ++s.index;
            }
            else if constexpr (std::same_as<S_t, ObjectState>) {
                ++s.iter;
            }
            // Scalar: no-op (empty range, ++ on end stays stable).
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
        std::visit([]<class S>(S& s) {
            using S_t = std::remove_cvref_t<S>;
            if constexpr (std::same_as<S_t, ArrayState>) {
                --s.iter; --s.index;
            }
            else if constexpr (std::same_as<S_t, ObjectState>) {
                --s.iter;
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
    auto JsonItemsView<IsConst>::begin() const -> iterator {
        return iterator{ m_parent, false };
    }

    template<bool IsConst>
    auto JsonItemsView<IsConst>::end() const -> iterator {
        return iterator{ m_parent, true };
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

}  // namespace Thoth::NJson

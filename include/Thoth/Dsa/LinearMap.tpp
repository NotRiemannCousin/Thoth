#pragma once
#include <algorithm>

#include <Hermes/Utils/Hash.hpp>

namespace Thoth::Dsa {
    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    constexpr LinearMap<KeyT, ValT, Pred, Alloc>::LinearMap(const key_compare& comp, const Alloc& alloc)
        : m_data{ alloc }, m_compare{ comp } {}

    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    constexpr LinearMap<KeyT, ValT, Pred, Alloc>::LinearMap(const Alloc& alloc)
        : LinearMap(key_compare{}, alloc) {}

    template<class KeyT, class ValT, class Pred, class Alloc> requires strong_order_relation<KeyT, Pred>
    constexpr LinearMap<KeyT, ValT, Pred, Alloc>::LinearMap(container_type&& init, const key_compare& comp)
        : m_data{ std::move(init) }, m_compare{ comp }{
        const auto less{ [this](const auto& a, const auto& b) { return key_less(a, b); } };

        if (!std::ranges::is_sorted(m_data, less, &value_type::first))
            std::ranges::stable_sort(m_data, less, &value_type::first);

        const auto key_equiv{ [this](const KeyT& a, const KeyT& b) {
            return compare_keys(a, b) == 0;
        } };

        auto [firstToErase, last]{ std::ranges::unique(m_data, key_equiv, &value_type::first) };
        m_data.erase(firstToErase, last);
    }

    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    template<std::ranges::input_range R>
    constexpr LinearMap<KeyT, ValT, Pred, Alloc>::LinearMap(std::from_range_t, R&& rg, const key_compare& comp, const Alloc& alloc)
        requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
        : LinearMap(container_type{ std::ranges::begin(rg), std::ranges::end(rg), alloc }, comp) {
    }

    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    template<std::ranges::input_range R>
    constexpr LinearMap<KeyT, ValT, Pred, Alloc>::LinearMap(std::from_range_t, R&& rg, const Alloc& alloc)
        requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>
        : LinearMap(std::from_range, std::forward<R>(rg), key_compare{}, alloc) {
    }

    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    constexpr LinearMap<KeyT, ValT, Pred, Alloc>::LinearMap(std::initializer_list<value_type> init, const key_compare& comp, const Alloc& alloc)
        : LinearMap(std::from_range, init, comp, alloc) {}

    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    constexpr LinearMap<KeyT, ValT, Pred, Alloc>::LinearMap(std::initializer_list<value_type> init, const Alloc& alloc)
        : LinearMap(std::from_range, init, key_compare{}, alloc) {}


    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred, Alloc>::allocator_type
    LinearMap<KeyT, ValT, Pred, Alloc>::get_allocator() const noexcept {
        return m_data.get_allocator();
    }

    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    constexpr bool LinearMap<KeyT, ValT, Pred, Alloc>::operator==(const LinearMap& other) const {
        return m_data == other.m_data;
    }

    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    constexpr void LinearMap<KeyT, ValT, Pred, Alloc>::clear() {
        m_data.clear();
    }

    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    template<class A, class B>
    constexpr std::strong_ordering LinearMap<KeyT, ValT, Pred, Alloc>::compare_keys(const A& a, const B& b) const {
        if constexpr (is_three_way_v<A, B>) {
            return std::invoke(m_compare, a, b);
        } else {
            if (std::invoke(m_compare, a, b)) return std::strong_ordering::less;
            if (std::invoke(m_compare, b, a)) return std::strong_ordering::greater;
            return std::strong_ordering::equal;
        }
    }

    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    template<class A, class B>
    constexpr bool LinearMap<KeyT, ValT, Pred, Alloc>::key_less(const A& a, const B& b) const {
        if constexpr (is_three_way_v<A, B>)
            return std::invoke(m_compare, a, b) < 0;
        else
            return static_cast<bool>(std::invoke(m_compare, a, b));
    }

    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT>
    constexpr std::pair<typename LinearMap<KeyT, ValT, Pred, Alloc>::const_iterator, bool>
    LinearMap<KeyT, ValT, Pred, Alloc>::find_position(const LookupKeyT& key) const {
        const auto first{ m_data.begin() };
        const auto last { m_data.end() };

        if (m_data.size() < 16) {
            // Ordenado: para na primeira chave que não é menor. 1 comparação por elemento.
            for (auto it{ first }; it != last; ++it) {
                const auto c{ compare_keys(it->first, key) };
                if (c >= 0)
                    return { it, c == 0 };
            }
            return { last, false };
        }

        // Busca binária com saída antecipada (chaves únicas): 1 comparação por passo.
        size_type lo{ 0 };
        size_type hi{ m_data.size() };
        while (lo < hi) {
            const size_type mid{ lo + (hi - lo) / 2 };
            const auto c{ compare_keys(m_data[mid].first, key) };
            if (c == 0) return { first + static_cast<std::ptrdiff_t>(mid), true };
            if (c < 0)  lo = mid + 1;
            else        hi = mid;
        }
        return { first + static_cast<std::ptrdiff_t>(lo), false };
    }

    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT>
    constexpr std::pair<typename LinearMap<KeyT, ValT, Pred, Alloc>::iterator, bool>
    LinearMap<KeyT, ValT, Pred, Alloc>::find_position(const LookupKeyT& key) {
        const auto [cit, found]{ std::as_const(*this).find_position(key) };
        return { m_data.begin() + (cit - m_data.cbegin()), found };
    }

    template<class KeyT, class ValT, class Pred, class Alloc> requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred, Alloc>::iterator       LinearMap<KeyT, ValT, Pred, Alloc>::begin()  { return m_data.begin(); }
    template<class KeyT, class ValT, class Pred, class Alloc> requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred, Alloc>::iterator       LinearMap<KeyT, ValT, Pred, Alloc>::end()    { return m_data.end(); }
    template<class KeyT, class ValT, class Pred, class Alloc> requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred, Alloc>::const_iterator LinearMap<KeyT, ValT, Pred, Alloc>::begin() const { return m_data.cbegin(); }
    template<class KeyT, class ValT, class Pred, class Alloc> requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred, Alloc>::const_iterator LinearMap<KeyT, ValT, Pred, Alloc>::end() const   { return m_data.cend(); }
    template<class KeyT, class ValT, class Pred, class Alloc> requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred, Alloc>::const_iterator LinearMap<KeyT, ValT, Pred, Alloc>::cbegin() const { return m_data.cbegin(); }
    template<class KeyT, class ValT, class Pred, class Alloc> requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred, Alloc>::const_iterator LinearMap<KeyT, ValT, Pred, Alloc>::cend() const   { return m_data.cend(); }

    template<class KeyT, class ValT, class Pred, class Alloc> requires strong_order_relation<KeyT, Pred>
    constexpr bool LinearMap<KeyT, ValT, Pred, Alloc>::empty() const { return m_data.empty(); }
    template<class KeyT, class ValT, class Pred, class Alloc> requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred, Alloc>::size_type LinearMap<KeyT, ValT, Pred, Alloc>::size() const { return m_data.size(); }

    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT, class MappedT>
    constexpr std::pair<typename LinearMap<KeyT, ValT, Pred, Alloc>::iterator, bool>
    LinearMap<KeyT, ValT, Pred, Alloc>::try_emplace(LookupKeyT&& key, MappedT&& val) {
        auto [it, found]{ find_position(key) };

        if (found) {
            return {it, false};
        }

        const iterator new_it{ m_data.emplace(it, KeyT{ std::forward<LookupKeyT>(key) }, std::forward<MappedT>(val)) };
        return {new_it, true};
    }

    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT, class MappedT>
    constexpr std::pair<typename LinearMap<KeyT, ValT, Pred, Alloc>::iterator, bool>
    LinearMap<KeyT, ValT, Pred, Alloc>::insert_or_assign(LookupKeyT&& key, MappedT&& val) {
        auto [it, found]{ find_position(key) };

        if (found) {
            it->second = std::forward<MappedT>(val);
            return {it, false};
        }

        const iterator new_it{ m_data.emplace(it, KeyT{ std::forward<LookupKeyT>(key) }, std::forward<MappedT>(val)) };
        return {new_it, true};
    }

    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT>
    constexpr bool LinearMap<KeyT, ValT, Pred, Alloc>::erase(const LookupKeyT& key) {
        auto [it, found]{ find_position(key) };

        if (!found) return false;

        m_data.erase(it);
        return true;
    }

    template<class KeyT, class ValT, class Pred, class Alloc> requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred, Alloc>::iterator LinearMap<KeyT, ValT, Pred, Alloc>::erase(iterator pos) { return m_data.erase(pos); }
    template<class KeyT, class ValT, class Pred, class Alloc> requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred, Alloc>::iterator LinearMap<KeyT, ValT, Pred, Alloc>::erase(const_iterator pos) { return m_data.erase(pos); }

    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT>
    constexpr typename LinearMap<KeyT, ValT, Pred, Alloc>::iterator LinearMap<KeyT, ValT, Pred, Alloc>::find(const LookupKeyT& key) {
        auto [it, found]{ find_position(key) };
        return found ? it : m_data.end();
    }

    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT>
    constexpr typename LinearMap<KeyT, ValT, Pred, Alloc>::const_iterator LinearMap<KeyT, ValT, Pred, Alloc>::find(const LookupKeyT& key) const {
        auto [it, found]{ find_position(key) };
        return found ? it : m_data.end();
    }

    template<class KeyT, class ValT, class Pred, class Alloc> requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT>
    constexpr bool LinearMap<KeyT, ValT, Pred, Alloc>::exists(const LookupKeyT& key) const { return find_position(key).second; }
    template<class KeyT, class ValT, class Pred, class Alloc> requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT>
    constexpr bool LinearMap<KeyT, ValT, Pred, Alloc>::contains(const LookupKeyT& key) const { return exists(key); }

    template<class KeyT, class ValT, class Pred, class Alloc>
        requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT>
    ValT& LinearMap<KeyT, ValT, Pred, Alloc>::operator[](LookupKeyT&& key) {
        auto [it, found]{ find_position(key) };

        if (found)
            return it->second;

        auto newIt{ m_data.emplace(it, KeyT{std::forward<LookupKeyT>(key)}, mapped_type{}) };
        return newIt->second;
   }
}


template<class K, class V, class P, class A>
    requires requires(const K& k){ std::hash<K>{}(k); } && requires(const V& v){ std::hash<V>{}(v); }
struct std::hash<Thoth::Dsa::LinearMap<K,V,P,A>> {
    size_t operator()(const Thoth::Dsa::LinearMap<K,V,P,A>& m) const noexcept {
        using Hermes::Utils::HashCombine;
        size_t seed{ 1469598103934665603ULL };

        for (const auto& p : m) {
            HashCombine(seed, std::hash<K>{}(p.first));
            HashCombine(seed, std::hash<V>{}(p.second));
        }
        HashCombine(seed, std::hash<size_t>{}(m.size()));
        return seed;
    }
};

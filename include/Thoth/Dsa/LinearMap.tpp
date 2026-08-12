#pragma once
#include <algorithm>

#include <Hermes/Utils/Hash.hpp>

namespace Thoth::Dsa {
    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    constexpr LinearMap<KeyT, ValT, Pred>::LinearMap(const key_compare& comp)
        : m_compare(comp) {}

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    constexpr LinearMap<KeyT, ValT, Pred>::LinearMap(std::initializer_list<value_type> init, const key_compare& comp)
        : m_data(init), m_compare(comp) {
        std::ranges::sort(m_data, m_compare, &value_type::first);

        const auto key_equiv{ [this](const KeyT& a, const KeyT& b) {
            return !std::invoke(m_compare, a, b) && !std::invoke(m_compare, b, a);
        } };

        auto [firstToErase, last]{ std::ranges::unique(m_data, key_equiv, &value_type::first) };
        m_data.erase(firstToErase, last);
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    constexpr bool LinearMap<KeyT, ValT, Pred>::operator==(const LinearMap& other) const {
        return m_data == other.m_data;
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    constexpr void LinearMap<KeyT, ValT, Pred>::clear() {
        m_data.clear();
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT>
    typename LinearMap<KeyT, ValT, Pred>::iterator
    constexpr LinearMap<KeyT, ValT, Pred>::find_position(const LookupKeyT& key) {
        return std::ranges::lower_bound(m_data, key, m_compare, &value_type::first);
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT>
    typename LinearMap<KeyT, ValT, Pred>::const_iterator
    constexpr LinearMap<KeyT, ValT, Pred>::find_position(const LookupKeyT& key) const {
        return std::ranges::lower_bound(m_data, key, m_compare, &value_type::first);
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT>
    constexpr bool LinearMap<KeyT, ValT, Pred>::is_equivalent(const_iterator it, const LookupKeyT& key) const {
        if (it == m_data.end()) return false;
        return !std::invoke(m_compare, key, it->first);
    }

    template<class KeyT, class ValT, class Pred> requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred>::iterator       LinearMap<KeyT, ValT, Pred>::begin()  { return m_data.begin(); }
    template<class KeyT, class ValT, class Pred> requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred>::iterator       LinearMap<KeyT, ValT, Pred>::end()    { return m_data.end(); }
    template<class KeyT, class ValT, class Pred> requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred>::const_iterator LinearMap<KeyT, ValT, Pred>::begin() const { return m_data.cbegin(); }
    template<class KeyT, class ValT, class Pred> requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred>::const_iterator LinearMap<KeyT, ValT, Pred>::end() const   { return m_data.cend(); }
    template<class KeyT, class ValT, class Pred> requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred>::const_iterator LinearMap<KeyT, ValT, Pred>::cbegin() const { return m_data.cbegin(); }
    template<class KeyT, class ValT, class Pred> requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred>::const_iterator LinearMap<KeyT, ValT, Pred>::cend() const   { return m_data.cend(); }

    template<class KeyT, class ValT, class Pred> requires strong_order_relation<KeyT, Pred>
    constexpr bool LinearMap<KeyT, ValT, Pred>::empty() const { return m_data.empty(); }
    template<class KeyT, class ValT, class Pred> requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred>::size_type LinearMap<KeyT, ValT, Pred>::size() const { return m_data.size(); }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT, class MappedT>
    constexpr std::pair<typename LinearMap<KeyT, ValT, Pred>::iterator, bool>
    LinearMap<KeyT, ValT, Pred>::try_emplace(LookupKeyT&& key, MappedT&& val) {
        iterator it{ find_position(key) };

        if (is_equivalent(it, key)) {
            return {it, false};
        }

        const iterator new_it{ m_data.emplace(it, KeyT{ std::forward<LookupKeyT>(key) }, std::forward<MappedT>(val)) };
        return {new_it, true};
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT, class MappedT>
    constexpr std::pair<typename LinearMap<KeyT, ValT, Pred>::iterator, bool>
    LinearMap<KeyT, ValT, Pred>::insert_or_assign(LookupKeyT&& key, MappedT&& val) {
        iterator it{ find_position(key) };

        if (is_equivalent(it, key)) {
            it->second = std::forward<MappedT>(val);
            return {it, false};
        }

        const iterator new_it{ m_data.emplace(it, KeyT{ std::forward<LookupKeyT>(key) }, std::forward<MappedT>(val)) };
        return {new_it, true};
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT>
    constexpr bool LinearMap<KeyT, ValT, Pred>::erase(const LookupKeyT& key) {
        auto it{ find_position(key) };

        if (!is_equivalent(it, key)) return false;

        m_data.erase(it);
        return true;
    }

    template<class KeyT, class ValT, class Pred> requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred>::iterator LinearMap<KeyT, ValT, Pred>::erase(iterator pos) { return m_data.erase(pos); }
    template<class KeyT, class ValT, class Pred> requires strong_order_relation<KeyT, Pred>
    constexpr typename LinearMap<KeyT, ValT, Pred>::iterator LinearMap<KeyT, ValT, Pred>::erase(const_iterator pos) { return m_data.erase(pos); }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT>
    constexpr typename LinearMap<KeyT, ValT, Pred>::iterator LinearMap<KeyT, ValT, Pred>::find(const LookupKeyT& key) {
        iterator it{ find_position(key) };
        return is_equivalent(it, key) ? it : m_data.end();
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT>
    constexpr typename LinearMap<KeyT, ValT, Pred>::const_iterator LinearMap<KeyT, ValT, Pred>::find(const LookupKeyT& key) const {
        const_iterator it{ find_position(key) };
        return is_equivalent(it, key) ? it : m_data.end();
    }

    template<class KeyT, class ValT, class Pred> requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT>
    constexpr bool LinearMap<KeyT, ValT, Pred>::exists(const LookupKeyT& key) const { return is_equivalent(find_position(key), key); }
    template<class KeyT, class ValT, class Pred> requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT>
    constexpr bool LinearMap<KeyT, ValT, Pred>::contains(const LookupKeyT& key) const { return exists(key); }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT>
    ValT& LinearMap<KeyT, ValT, Pred>::operator[](LookupKeyT&& key) {
        auto it{ find_position(key) };

        if (is_equivalent(it, key))
            return it->second;

        auto newIt{ m_data.emplace(it, KeyT{std::forward<LookupKeyT>(key)}, mapped_type{}) };
        return newIt->second;
   }
}


template<class K, class V, class P>
    requires requires(const K& k){ std::hash<K>{}(k); } && requires(const V& v){ std::hash<V>{}(v); }
struct std::hash<Thoth::Dsa::LinearMap<K,V,P>> {
    size_t operator()(const Thoth::Dsa::LinearMap<K,V,P>& m) const noexcept {
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
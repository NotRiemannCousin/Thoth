#pragma once
#include <algorithm>

#include <Thoth/Utils/Hash.hpp>

namespace Thoth::Dsa {
    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    constexpr LinearMap<KeyT, ValT, Pred>::LinearMap(const key_compare& comp)
        : _compare(comp) {}


    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    constexpr LinearMap<KeyT, ValT, Pred>::LinearMap(std::initializer_list<value_type> init, const key_compare& comp)
    : _data(init), _compare(comp) {

        auto pair_comp = [this](const value_type& a, const value_type& b) {
            return std::invoke(_compare, a.first, b.first);
        };

        std::sort(_data.begin(), _data.end(), pair_comp);

        auto key_equiv = [this](const value_type& a, const value_type& b) {
            return !std::invoke(_compare, a.first, b.first) && !std::invoke(_compare, b.first, a.first);
        };

        const auto last{ std::unique(_data.begin(), _data.end(), key_equiv) };

        _data.erase(last, _data.end());
    }

    template<class KeyT, class ValT, class Pred> requires strong_order_relation<KeyT, Pred>
    constexpr LinearMap<KeyT, ValT, Pred> & LinearMap<KeyT, ValT, Pred>::operator=(const LinearMap & other) {
        _compare = other._compare;
        _data = other._data;
        return *this;
    }

    template<class KeyT, class ValT, class Pred> requires strong_order_relation<KeyT, Pred>
    constexpr LinearMap<KeyT, ValT, Pred> & LinearMap<KeyT, ValT, Pred>::operator=(LinearMap && other) noexcept {
        _compare = std::move(other._compare);
        _data = std::move(other._data);
        return *this;
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    constexpr bool LinearMap<KeyT, ValT, Pred>::operator==(const LinearMap& other) const {
        return _data == other._data;
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    constexpr void LinearMap<KeyT, ValT, Pred>::clear() {
        _data.clear();
    }


    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template <class LookupKeyT>
    typename LinearMap<KeyT, ValT, Pred>::iterator
    constexpr  LinearMap<KeyT, ValT, Pred>::find_position(const LookupKeyT& key) {
        return std::lower_bound(_data.begin(), _data.end(), key, PairComparator{ _compare });
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template <class LookupKeyT>
    typename LinearMap<KeyT, ValT, Pred>::const_iterator
    constexpr  LinearMap<KeyT, ValT, Pred>::find_position(const LookupKeyT& key) const {
        return std::lower_bound(_data.cbegin(), _data.cend(), key, PairComparator{ _compare });
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template <class LookupKeyT>
    constexpr bool LinearMap<KeyT, ValT, Pred>::is_equivalent(const_iterator it, const LookupKeyT& key) const {
        if (it == _data.end()) {
            return false;
        }
        return !std::invoke(_compare, key, it->first);
    }


    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    typename LinearMap<KeyT, ValT, Pred>::iterator
    constexpr  LinearMap<KeyT, ValT, Pred>::begin() {
        return _data.begin();
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    typename LinearMap<KeyT, ValT, Pred>::iterator
    constexpr  LinearMap<KeyT, ValT, Pred>::end() {
        return _data.end();
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    typename LinearMap<KeyT, ValT, Pred>::const_iterator
    constexpr  LinearMap<KeyT, ValT, Pred>::begin() const {
        return _data.cbegin();
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    typename LinearMap<KeyT, ValT, Pred>::const_iterator
    constexpr  LinearMap<KeyT, ValT, Pred>::end() const {
        return _data.cend();
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    typename LinearMap<KeyT, ValT, Pred>::const_iterator
    constexpr  LinearMap<KeyT, ValT, Pred>::cbegin() const {
        return _data.cbegin();
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    typename LinearMap<KeyT, ValT, Pred>::const_iterator
    constexpr  LinearMap<KeyT, ValT, Pred>::cend() const {
        return _data.cend();
    }


    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    constexpr bool LinearMap<KeyT, ValT, Pred>::empty() const {
        return _data.empty();
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    typename LinearMap<KeyT, ValT, Pred>::size_type
    constexpr LinearMap<KeyT, ValT, Pred>::size() const {
        return _data.size();
    }


    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template <class LookupKeyT, class MappedT>
    std::pair<class LinearMap<KeyT, ValT, Pred>::iterator, bool>
    constexpr LinearMap<KeyT, ValT, Pred>::try_emplace(LookupKeyT&& key, MappedT&& val) {
        const iterator it{ find_position(key) };

        if (is_equivalent(it, key)) {
            return {it, false};
        }

        const iterator new_it{ _data.emplace(it, KeyT{ std::forward<LookupKeyT>(key) }, std::forward<MappedT>(val)) };
        return {new_it, true};
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template <class LookupKeyT, class MappedT>
    std::pair<typename LinearMap<KeyT, ValT, Pred>::iterator, bool>
    constexpr LinearMap<KeyT, ValT, Pred>::insert_or_assign(LookupKeyT&& key, MappedT&& val) {
        iterator it{ find_position(key) };

        if (is_equivalent(it, key)) {
            it->second = std::forward<MappedT>(val);
            return {it, false};
        }

        const iterator new_it{ _data.emplace(it, KeyT{ std::forward<LookupKeyT>(key) }, std::forward<MappedT>(val)) };
        return {new_it, true};
    }

    template<class KeyT, class ValT, class Pred>
    requires strong_order_relation<KeyT, Pred>
template <class LookupKeyT>
constexpr bool LinearMap<KeyT, ValT, Pred>::erase(const LookupKeyT& key) {
        auto it{ find_position(key) };

        if (!is_equivalent(it, key))
            return false;

        _data.erase(it);
        return true;
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    typename LinearMap<KeyT, ValT, Pred>::iterator
    constexpr LinearMap<KeyT, ValT, Pred>::erase(iterator pos) {
        return _data.erase(pos);
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    typename LinearMap<KeyT, ValT, Pred>::iterator
    constexpr LinearMap<KeyT, ValT, Pred>::erase(const_iterator pos) {
        return _data.erase(pos);
    }


    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template <class LookupKeyT>
    typename LinearMap<KeyT, ValT, Pred>::iterator
    constexpr  LinearMap<KeyT, ValT, Pred>::find(const LookupKeyT& key) {
        const iterator it{ find_position(key) };
        if (is_equivalent(it, key)) {
            return it;
        }
        return _data.end();
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template <class LookupKeyT>
    typename LinearMap<KeyT, ValT, Pred>::const_iterator
    constexpr  LinearMap<KeyT, ValT, Pred>::find(const LookupKeyT& key) const {
        const_iterator it{ find_position(key) };
        if (is_equivalent(it, key)) {
            return it;
        }
        return _data.end();
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template <class LookupKeyT>
    constexpr bool LinearMap<KeyT, ValT, Pred>::exists(const LookupKeyT& key) const {
        const_iterator it{ find_position(key) };
        return is_equivalent(it, key);
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template <class LookupKeyT>
    constexpr bool LinearMap<KeyT, ValT, Pred>::contains(const LookupKeyT& key) const {
        return exists(key);
    }

    template<class KeyT, class ValT, class Pred> requires strong_order_relation<KeyT, Pred>
    template<class LookupKeyT>
    ValT& LinearMap<KeyT, ValT, Pred>::operator[](LookupKeyT&& key) {
        auto it{ find_position(key) };
        if (is_equivalent(it, key))
            return it->second;

        auto [newIt, _] = try_emplace(KeyT{std::forward<LookupKeyT>(key)}, mapped_type{});
        return newIt->second;

   }
}


template<class K, class V, class P>
    requires requires(const K& k){ std::hash<K>{}(k); } && requires(const V& v){ std::hash<V>{}(v); }
struct std::hash<Thoth::Dsa::LinearMap<K,V,P>> {
    size_t operator()(const Thoth::Dsa::LinearMap<K,V,P>& m) const noexcept {
        using Thoth::Utils::HashCombine;
        size_t seed{ 1469598103934665603ULL };

        for (const auto& p : m) {
            HashCombine(seed, std::hash<K>{}(p.first));
            HashCombine(seed, std::hash<V>{}(p.second));
        }
        HashCombine(seed, std::hash<size_t>{}(m.size()));
        return seed;
    }
};
#pragma once
#include <algorithm>
#include <map>


namespace Thoth::Dsa {
    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
   constexpr  LinearMap<KeyT, ValT, Pred>::LinearMap(const key_compare& comp)
        : _compare(comp) {}


    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
   constexpr  LinearMap<KeyT, ValT, Pred>::LinearMap(std::initializer_list<value_type> init, const key_compare& comp)
    : _data(init), _compare(comp) {

        auto pair_comp = [this](const value_type& a, const value_type& b) {
            return std::invoke(_compare, a.first, b.first);
        };

        std::sort(_data.begin(), _data.end(), pair_comp);

        auto key_equiv = [this](const value_type& a, const value_type& b) {
            return !std::invoke(_compare, a.first, b.first) && !std::invoke(_compare, b.first, a.first);
        };

        auto last = std::unique(_data.begin(), _data.end(), key_equiv);
        _data.erase(last, _data.end());
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
    template <typename LookupKeyT>
    typename LinearMap<KeyT, ValT, Pred>::iterator
   constexpr  LinearMap<KeyT, ValT, Pred>::find_position(const LookupKeyT& key) {
        return std::lower_bound(_data.begin(), _data.end(), key, PairComparator{_compare});
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template <typename LookupKeyT>
    typename LinearMap<KeyT, ValT, Pred>::const_iterator
   constexpr  LinearMap<KeyT, ValT, Pred>::find_position(const LookupKeyT& key) const {
        return std::lower_bound(_data.cbegin(), _data.cend(), key, PairComparator{_compare});
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template <typename LookupKeyT>
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
    template <typename LookupKeyT, typename MappedT>
    std::pair<typename LinearMap<KeyT, ValT, Pred>::iterator, bool>
   constexpr LinearMap<KeyT, ValT, Pred>::try_emplace(const LookupKeyT& key, MappedT&& val) {
        iterator it = find_position(key);

        if (is_equivalent(it, key)) {
            return {it, false};
        }

        iterator new_it = _data.emplace(it, key, std::forward<MappedT>(val));
        return {new_it, true};
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template <typename LookupKeyT>
    typename LinearMap<KeyT, ValT, Pred>::size_type
   constexpr  LinearMap<KeyT, ValT, Pred>::erase(const LookupKeyT& key) {
        iterator it = find(key);
        if (it == _data.end()) {
            return 0;
        }

        _data.erase(it);
        return 1;
    }


    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template <typename LookupKeyT>
    typename LinearMap<KeyT, ValT, Pred>::iterator
   constexpr  LinearMap<KeyT, ValT, Pred>::find(const LookupKeyT& key) {
        iterator it = find_position(key);
        if (is_equivalent(it, key)) {
            return it;
        }
        return _data.end();
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template <typename LookupKeyT>
    typename LinearMap<KeyT, ValT, Pred>::const_iterator
   constexpr  LinearMap<KeyT, ValT, Pred>::find(const LookupKeyT& key) const {
        const_iterator it = find_position(key);
        if (is_equivalent(it, key)) {
            return it;
        }
        return _data.end();
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template <typename LookupKeyT>
    constexpr bool LinearMap<KeyT, ValT, Pred>::exists(const LookupKeyT& key) const {
        return find(key) != _data.end();
    }

    template<class KeyT, class ValT, class Pred>
        requires strong_order_relation<KeyT, Pred>
    template <typename LookupKeyT>
    constexpr bool LinearMap<KeyT, ValT, Pred>::contains(const LookupKeyT& key) const {
        return exists(key);
    }

}

// esse deixei pra IA fds, mó preguiça de fazer
#pragma once

#include <vector>
#include <utility>
#include <concepts>
#include <functional>
#include <compare>

namespace Thoth::Dsa {

    template <class Key, class Relation>
    concept strong_order_relation =
           requires(Relation r, Key a, Key b) { { std::invoke(r, a, b) } -> std::same_as<std::strong_ordering>; }
        || requires(Relation r, Key a, Key b) { { std::invoke(r, a, b) } -> std::convertible_to<bool>; };

    template<class KeyT, class ValT, class Pred = std::less<>>
        requires strong_order_relation<KeyT, Pred>
    struct LinearMap {
        using key_type       = KeyT;
        using mapped_type    = ValT;
        using value_type     = std::pair<KeyT, ValT>;
        using key_compare    = Pred;
        using container_type = std::vector<value_type>;
        using iterator       = container_type::iterator;
        using const_iterator = container_type::const_iterator;
        using size_type      = container_type::size_type;

    private:
        container_type _data;
        key_compare _compare;

        struct PairComparator {
            const key_compare& _comp;

            template <typename LookupKeyT>
            constexpr bool operator()(const value_type& pair, const LookupKeyT& key) const {
                return std::invoke(_comp, pair.first, key);
            }

            template <typename LookupKeyT>
            constexpr bool operator()(const LookupKeyT& key, const value_type& pair) const {
                return std::invoke(_comp, key, pair.first);
            }
        };

        template <typename LookupKeyT>
        constexpr iterator find_position(const LookupKeyT& key);

        template <typename LookupKeyT>
        constexpr const_iterator find_position(const LookupKeyT& key) const;

        template <typename LookupKeyT>
        constexpr bool is_equivalent(const_iterator it, const LookupKeyT& key) const;

    public:
        constexpr LinearMap() = default;
        constexpr LinearMap(const LinearMap&) = default;
        constexpr LinearMap(LinearMap&&) = default;

        constexpr explicit LinearMap(const key_compare& comp);
        constexpr LinearMap(std::initializer_list<value_type> init, const key_compare& comp = key_compare());


        constexpr LinearMap& operator=(const LinearMap&) = default;
        constexpr LinearMap& operator=(LinearMap&&) = default;

        constexpr bool operator==(const LinearMap& other) const;


        constexpr void clear();

        constexpr iterator begin();
        constexpr iterator end();
        constexpr const_iterator begin() const;
        constexpr const_iterator end() const;
        constexpr const_iterator cbegin() const;
        constexpr const_iterator cend() const;

        [[nodiscard]] constexpr bool empty() const;
        constexpr size_type size() const;

        template <typename LookupKeyT, typename MappedT>
        constexpr std::pair<iterator, bool> try_emplace(const LookupKeyT& key, MappedT&& val);

        template <typename LookupKeyT>
        constexpr size_type erase(const LookupKeyT& key);

        template <typename LookupKeyT>
        constexpr iterator find(const LookupKeyT& key);

        template <typename LookupKeyT>
        constexpr const_iterator find(const LookupKeyT& key) const;

        template <typename LookupKeyT>
        constexpr bool exists(const LookupKeyT& key) const;

        template <typename LookupKeyT>
        constexpr bool contains(const LookupKeyT& key) const;
    };
}

#include <Thoth/Dsa/LinearMap.tpp>

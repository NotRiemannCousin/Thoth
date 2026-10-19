#pragma once

#include <vector>
#include <memory>
#include <utility>
#include <concepts>
#include <functional>
#include <compare>
#include <cstddef>
#include <ranges>
#include <type_traits>

namespace Thoth::Dsa {

    template<class Key, class Relation>
    concept strong_order_relation =
           requires(Relation r, Key a, Key b) { { std::invoke(r, a, b) } -> std::same_as<std::strong_ordering>; }
        || requires(Relation r, Key a, Key b) { { std::invoke(r, a, b) } -> std::convertible_to<bool>; };

    template<class KeyT, class ValT, class Pred = std::less<>,
             class Alloc = std::allocator<std::pair<KeyT, ValT>>>
        requires strong_order_relation<KeyT, Pred>
    struct LinearMap {
        using key_type       = KeyT;
        using mapped_type    = ValT;
        using value_type     = std::pair<KeyT, ValT>;
        using key_compare    = Pred;
        using allocator_type = Alloc;
        using container_type = std::vector<value_type, Alloc>;
        using iterator       = container_type::iterator;
        using const_iterator = container_type::const_iterator;
        using size_type      = container_type::size_type;

    private:
        container_type m_data;
        [[no_unique_address]] key_compare m_compare;

        template<class A, class B>
        static constexpr bool is_three_way_v =
            std::same_as<std::invoke_result_t<const key_compare&, const A&, const B&>, std::strong_ordering>;

        template<class A, class B>
        constexpr std::strong_ordering compare_keys(const A& a, const B& b) const;

        template<class A, class B>
        constexpr bool key_less(const A& a, const B& b) const;

        template<class LookupKeyT>
        constexpr std::pair<iterator, bool> find_position(const LookupKeyT& key);

        template<class LookupKeyT>
        constexpr std::pair<const_iterator, bool> find_position(const LookupKeyT& key) const;

    public:
        constexpr LinearMap() = default;
        constexpr LinearMap(const LinearMap&) = default;
        constexpr LinearMap(LinearMap&&) = default;

        constexpr explicit LinearMap(const key_compare& comp, const Alloc& alloc = Alloc{});
        constexpr explicit LinearMap(const Alloc& alloc);

        constexpr LinearMap(container_type&& init, const key_compare& comp = key_compare{});
        template<std::ranges::input_range R>
        constexpr LinearMap(std::from_range_t, R&& rg, const key_compare& comp = key_compare{}, const Alloc& alloc = Alloc{})
            requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>;
        template<std::ranges::input_range R>
        constexpr LinearMap(std::from_range_t, R&& rg, const Alloc& alloc)
            requires std::convertible_to<std::ranges::range_reference_t<R>, value_type>;
        constexpr LinearMap(std::initializer_list<value_type> init, const key_compare& comp = key_compare{}, const Alloc& alloc = Alloc{});
        constexpr LinearMap(std::initializer_list<value_type> init, const Alloc& alloc);

        constexpr LinearMap& operator=(const LinearMap&) = default;
        constexpr LinearMap& operator=(LinearMap&&) noexcept = default;

        [[nodiscard]] constexpr allocator_type get_allocator() const noexcept;

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

        template<class LookupKeyT, class MappedT>
        constexpr std::pair<iterator, bool> try_emplace(LookupKeyT&& key, MappedT&& val);

        template<class LookupKeyT, class MappedT>
        constexpr std::pair<iterator, bool> insert_or_assign(LookupKeyT&& key, MappedT&& val);

        template<class LookupKeyT>
        constexpr bool erase(const LookupKeyT& key);

        constexpr iterator erase(iterator pos);
        constexpr iterator erase(const_iterator pos);

        template<class LookupKeyT>
        constexpr iterator find(const LookupKeyT& key);

        template<class LookupKeyT>
        constexpr const_iterator find(const LookupKeyT& key) const;

        template<class LookupKeyT>
        constexpr bool exists(const LookupKeyT& key) const;

        template<class LookupKeyT>
        constexpr bool contains(const LookupKeyT& key) const;

        template<class LookupKeyT>
        ValT& operator[](LookupKeyT&& key);
    };
}

#include <Thoth/Dsa/LinearMap.tpp>

#pragma once
#include <optional>
#include <format>
#include <vector>
#include <string>
#include <map>

#include <Thoth/Dsa/LinearMap.hpp>

namespace Thoth::Http{
    struct QueryParams {
        using QueryKey      = std::string;
        using QueryKeyRef   = const std::string&;

        using QueryValue    = std::string;
        using QueryValueRef = const std::string&;
        using QueryValues   = std::vector<QueryValue>;

        using QueryPair     = std::pair<const QueryKey, QueryValues>;
        using MapType       = std::map<QueryKey, QueryValues>;
        // using MapType       = Dsa::LinearMap<QueryKey, QueryValues>;

        using IterType      = decltype(MapType().begin());
        using CIterType     = decltype(MapType().cbegin());


        //! @brief Create with an existing map.
        explicit QueryParams(const MapType& initAs);

        QueryParams(const std::initializer_list<QueryPair>& init);


        //! @brief Parse the query as it is.
        static QueryParams Parse(std::string_view paramsStr);
        //! @brief Tries to decode and then parse.
        static std::optional<QueryParams> ParseDecodified(std::string_view str);


        //! @brief check if a key exists.
        //! @param key The key to be checked.
        //! @return True if the key exists, false otherwise.
        [[nodiscard]] bool Exists(QueryKeyRef key) const;

        //! @brief check if a key=val exists.
        //! @param key The key to be checked.
        //! @param val The key to be checked.
        //! @return True if the key exists, false otherwise.
        [[nodiscard]] bool ValExists(QueryKeyRef key, QueryValueRef val) const;

        //! @brief Add a value with the specified key.
        //! @param key The key where it will be added.
        //! @param val The value to be added.
        void Add(QueryKeyRef key, QueryValueRef val);

        //! @brief Remove a value with the specified key.
        //! @param key The key where it will be removed.
        //! @param val The value to be removed.
        //! @return True if the key exists, false otherwise.
        bool Remove(QueryKeyRef key, QueryValueRef val);

        //! @brief Remove a key and all values associated.
        //! @param key The key that it will be removed.
        //! @return True if the key exists, false otherwise.
        bool RemoveKey(QueryKeyRef key);

        //! @brief If key not exists, set it to value.
        //! @param key The key.
        //! @param value The value to be added.
        //! @return True if the key not exists, false otherwise.
        bool SetIfNull(QueryKeyRef key, QueryValueRef value);
        //! @brief Get the reference of a key but don't create if it not exists.
        //! @param key The key.
        //! @return QueryValues* if the key exists, std::nullopt otherwise.
        std::optional<QueryValues*> Get(QueryKeyRef key);


        IterType begin();
        IterType end();
        [[nodiscard]] CIterType begin() const;
        [[nodiscard]] CIterType end() const;


        //! @brief Clear all keys.
        void Clear();

        //! @return The count of keys.
        [[nodiscard]] size_t Size() const;

        //! @return True if Size() is 0.
        [[nodiscard]] bool Empty() const;

        //! @return The QueryValues& associated with a key. Create if it not exists.
        //! STL containers has many problems so it must be QueryKey.
        QueryValues& operator[](QueryKeyRef key);

        //! @return True if both queries match.
        bool operator==(const QueryParams& other) const;
    private:
        MapType _elements;

        friend struct std::formatter<QueryParams>;
        friend struct std::hash<QueryParams>;
    };
}


#include <Thoth/Http/Request/QueryParams.tpp>

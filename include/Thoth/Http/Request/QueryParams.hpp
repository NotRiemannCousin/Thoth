#pragma once
#include <expected>
#include <format>
#include <vector>
#include <string>

#include <Thoth/Dsa/LinearMap.hpp>
#include <Thoth/ThothError.hpp>

namespace Thoth::Http{
    struct QueryParams {
        using QueryKey      = std::string;
        using QueryKeyRef   = std::string_view;

        using QueryValue    = std::string;
        using QueryValueRef = std::string_view;
        using QueryValues   = std::vector<QueryValue>;

        using QueryPair     = std::pair<QueryKey, QueryValues>;
        using MapType       = Dsa::LinearMap<QueryKey, QueryValues>;

        using IterType      = decltype(MapType().begin());
        using CIterType     = decltype(MapType().cbegin());


        //! @brief Create with an existing map.
        explicit QueryParams(const MapType& initAs);

        QueryParams(std::initializer_list<QueryPair> init);


        //! @brief Parse the query as it is.
        static std::expected<QueryParams, ThothError> Parse(std::string_view paramsStr);


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
        void Add(QueryKeyRef key, QueryValue val);

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

#pragma region Getters



        //! @brief Every value of a key.
        //! @return The values if the key exists, std::nullopt otherwise.
        std::optional<QueryValues*> Get(QueryKeyRef key);
        //! @copydoc Get(QueryKeyRef key)
        [[nodiscard]] std::optional<const QueryValues*> Get(QueryKeyRef key) const;

        //! @brief Every value of a key, if at least one of them is not empty.
        //! @brief The first value of a key if it exists.
        std::optional<QueryValue*> GetFront(QueryKeyRef key);
        //! @copydoc GetFront(QueryKeyRef key)
        [[nodiscard]] std::optional<const QueryValue*> GetFront(QueryKeyRef key) const;

        //! @return The values if the key exists and isn't empty, std::nullopt otherwise.
        std::optional<QueryValues*> GetNonEmpty(QueryKeyRef key);
        //! @copydoc GetNonEmpty(QueryKeyRef key)
        [[nodiscard]] std::optional<const QueryValues*> GetNonEmpty(QueryKeyRef key) const;

        //! @brief The first value of a key that is not empty.
        std::optional<QueryValue*> GetFrontNonEmpty(QueryKeyRef key);
        //! @copydoc GetFrontNonEmpty(QueryKeyRef key)
        [[nodiscard]] std::optional<const QueryValue*> GetFrontNonEmpty(QueryKeyRef key) const;

        //! @brief Every value of a key.
        //! @return The values if the key exists, std::nullopt otherwise.
        [[nodiscard]] std::optional<QueryValues> GetCopy(QueryKeyRef key) const;

        //! @brief Every value of a key, if at least one of them is not empty.
        //! @brief The first value of a key if it exists.
        [[nodiscard]] std::optional<QueryValue> GetCopyFront(QueryKeyRef key) const;

        //! @return The values if the key exists and isn't empty, std::nullopt otherwise.
        [[nodiscard]] std::optional<QueryValues> GetCopyNonEmpty(QueryKeyRef key) const;

        //! @brief The first value of a key that is not empty.
        [[nodiscard]] std::optional<QueryValue> GetCopyFrontNonEmpty(QueryKeyRef key) const;

#pragma endregion

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
        MapType m_elements;

        friend struct std::formatter<QueryParams>;
        friend struct std::hash<QueryParams>;
    };
}


#include <Thoth/Http/Request/QueryParams.tpp>

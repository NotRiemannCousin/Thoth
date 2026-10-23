#include <Thoth/Http/Request/QueryParams.hpp>
#include <Thoth/Http/Url/Url.hpp>

#include <algorithm>
#include <ranges>

namespace {
    template<class T>
    std::optional<T> NonEmpty(T val) {
        if constexpr (std::is_pointer_v<T>) {
            if (!val || val->empty()) return std::nullopt;
        } else {
            if (val.empty()) return std::nullopt;
        }
        return val;
    }

    template<class T>
    auto Front(T val) {
        if constexpr (std::is_pointer_v<T>) {
            using Pointee = std::remove_pointer_t<T>;
            using Elem = std::conditional_t< std::is_const_v<Pointee>,
                const typename Pointee::value_type,
                typename Pointee::value_type
            >;
            using Ret = std::optional<Elem*>;

            if (!val || val->empty()) return Ret{ std::nullopt };
            return Ret{ &val->front() };
        } else {
            using Val = typename T::value_type;
            using Ret = std::optional<typename T::value_type>;
            if (val.empty()) return Ret{ std::nullopt };
            return Ret{ val.front() };
        }
    }

    template<class T>
    T DecayPtr(const T* ptr) {
        return *ptr;
    }
}

namespace Thoth::Http {
    using std::string_view;
    using std::string;
    namespace rg = std::ranges;
    namespace vs = std::views;

    struct Url;

    QueryParams::QueryParams(const MapType& initAs): m_elements{ initAs } { }

    QueryParams::QueryParams(std::initializer_list<QueryPair> init)
            : m_elements{ init } {}

    std::expected<QueryParams, ThothError> QueryParams::Parse(std::string_view paramsStr) {
        QueryParams params{};
        for (const auto rawParam : paramsStr | vs::split('&')) {
            if (rawParam.empty())
                continue;

            const auto eqIt{ std::ranges::find(rawParam, '=') };
            const std::string_view rawKey{ rawParam.begin(), eqIt };
            const std::string_view rawVal{ eqIt == rawParam.end() ? eqIt : std::next(eqIt), rawParam.end() };

            auto decodedKey{ Url::TryDecode(rawKey) };
            auto decodedVal{ Url::TryDecode(rawVal) };

            if (!decodedKey || !decodedVal)
                return ThothUnex{ UrlParseErrorEnum::IllFormed };

        params[std::move(*decodedKey)].emplace_back(std::move(*decodedVal));
        }

        return params;
    }

    bool QueryParams::Exists(QueryKeyRef key) const { return m_elements.contains(key); }

    bool QueryParams::ValExists(QueryKeyRef key, QueryValueRef val) const {
        const auto&& it{ m_elements.find(key) };
        if (it == m_elements.end())
            return false;

        const auto&& it2{ std::ranges::find(it->second, val) };

        return it2 != it->second.end();
    }

    void QueryParams::Add(QueryKeyRef key, QueryValue val) {
        m_elements[key].push_back(val);
    }

    bool QueryParams::Remove(QueryKeyRef key, QueryValueRef val) {
        auto&& it = m_elements.find(key);
        if (it == m_elements.end())
            return false;

        auto& vec = it->second;
        const auto oldSize{ vec.size() };
        const auto newEnd{ std::ranges::remove(vec, val).begin() };
        vec.erase(newEnd, vec.end());

        return oldSize != vec.size();
    }

    bool QueryParams::RemoveKey(QueryKeyRef key) {
        auto&& it{ m_elements.find(key) };
        if (it == m_elements.end())
            return false;

        m_elements.erase(it);
        return true;
    }

    bool QueryParams::SetIfNull(QueryKeyRef key, QueryValueRef value) {
        if (Exists(key))
            return false;

        m_elements[key].emplace_back(value);
        return true;
    }

    std::optional<QueryParams::QueryValues*> QueryParams::Get(QueryKeyRef key) {
        if (const auto el{ m_elements.find(key) }; el != m_elements.end())
            return &el->second;

        return std::nullopt;
    }

    std::optional<const QueryParams::QueryValues*> QueryParams::Get(QueryKeyRef key) const {
        if (const auto el{ m_elements.find(key) }; el != m_elements.end())
            return &el->second;

        return std::nullopt;
    }

    std::optional<QueryParams::QueryValue*> QueryParams::GetFront(QueryKeyRef key) {
        return Get(key).and_then(Front<QueryValues*>);
    }

    std::optional<const QueryParams::QueryValue*> QueryParams::GetFront(QueryKeyRef key) const {
        return Get(key).and_then(Front<const QueryValues*>);
    }

    std::optional<QueryParams::QueryValues*> QueryParams::GetNonEmpty(QueryKeyRef key) {
        return Get(key).and_then(NonEmpty<QueryValues*>);
    }

    std::optional<const QueryParams::QueryValues*> QueryParams::GetNonEmpty(QueryKeyRef key) const {
        return Get(key).and_then(NonEmpty<const QueryValues*>);
    }

    std::optional<QueryParams::QueryValue*> QueryParams::GetFrontNonEmpty(QueryKeyRef key) {
        return GetFront(key).and_then(NonEmpty<QueryValue*>);
    }

    std::optional<const QueryParams::QueryValue*> QueryParams::GetFrontNonEmpty(QueryKeyRef key) const {
        return GetFront(key).and_then(NonEmpty<const QueryValue*>);
    }



    std::optional<QueryParams::QueryValues> QueryParams::GetCopy(QueryKeyRef key) const {
        return Get(key).transform(DecayPtr<QueryValues>);
    }

    std::optional<QueryParams::QueryValue> QueryParams::GetCopyFront(QueryKeyRef key) const {
        return GetFront(key).transform(DecayPtr<QueryValue>);
    }

    std::optional<QueryParams::QueryValues> QueryParams::GetCopyNonEmpty(QueryKeyRef key) const {
        return GetNonEmpty(key).transform(DecayPtr<QueryValues>);
    }

    std::optional<QueryParams::QueryValue> QueryParams::GetCopyFrontNonEmpty(QueryKeyRef key) const {
        return GetFrontNonEmpty(key).transform(DecayPtr<QueryValue>);
    }



    QueryParams::IterType QueryParams::begin() { return m_elements.begin(); }

    QueryParams::IterType QueryParams::end() { return m_elements.end(); }

    QueryParams::CIterType QueryParams::begin() const { return m_elements.cbegin(); }

    QueryParams::CIterType QueryParams::end() const { return m_elements.cend(); }

    void QueryParams::Clear() { m_elements.clear(); }

    size_t QueryParams::Size() const { return m_elements.size(); }

    bool QueryParams::Empty() const { return m_elements.size() == 0; }

    QueryParams::QueryValues& QueryParams::operator[](QueryKeyRef key) { return m_elements[key]; }

    bool QueryParams::operator==(const QueryParams& other) const = default;
}

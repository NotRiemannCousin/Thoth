#include <Thoth/Http/Request/QueryParams.hpp>
#include <Thoth/Http/Url/Url.hpp>

#include <algorithm>
#include <ranges>


namespace Thoth::Http {
    using std::string_view;
    using std::string;
    namespace rg = std::ranges;
    namespace vs = std::views;

    struct Url;

    QueryParams::QueryParams(const MapType& initAs): m_elements{ initAs } { }

    QueryParams::QueryParams(std::initializer_list<QueryPair> init)
            : m_elements{ init } {}

    QueryParams QueryParams::Parse(std::string_view paramsStr) {
        static constexpr auto splitDelimiter = [](char c) { return c != '='; };
        static constexpr auto splitBetween = [](const auto& str) {
            return std::pair{
                str | vs::take_while(splitDelimiter) | std::ranges::to<std::string>(),
                str | vs::drop_while(splitDelimiter) | vs::drop(1) | std::ranges::to<std::string>()
            };
        };

        QueryParams params{};
        for (const auto rawParam : paramsStr | vs::split('&')) {
            const auto [r, l] = splitBetween(rawParam);

            params[r].emplace_back(l);
        }

        return params;
    }

    std::expected<QueryParams, ExchangeError> QueryParams::ParseDecodified(std::string_view str) {
        return Url::TryDecode(str).transform(Parse);
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
        if (Exists(key))
            return &m_elements[key];

        return std::nullopt;
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

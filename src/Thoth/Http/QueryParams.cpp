#include <Thoth/Http/QueryParams.hpp>
#include <Thoth/Http/HttpUrl.hpp>

#include <algorithm>
#include <ranges>


namespace Thoth::Http {
    using std::string_view;
    using std::string;
    using std::map;
    namespace rg = std::ranges;
    namespace vs = std::views;

    struct HttpUrl;

    QueryParams::QueryParams(const MapType& initAs): _elements{ initAs } { }

    QueryParams::QueryParams(const std::initializer_list<QueryPair> &init)
            : _elements{ init } {}

    QueryParams QueryParams::Parse(std::string_view str) {
        static auto const ToStr = [](auto&& r) -> std::string {
            return r | rg::to<std::string>();
        };
        static auto const splitDelimiter = [](char c) { return c != '='; };
        static auto const splitBetween = [](const auto& str) {
            return std::pair{
                str | vs::take_while(splitDelimiter),
                str | vs::drop_while(splitDelimiter) | vs::drop(1)
            };
        };

        static auto const splitParams = [](const auto& rawParams) {
            auto [r, l] = splitBetween(rawParams);

            return std::pair{ r | rg::to<string>(), l | vs::split(',') | vs::transform(ToStr) | rg::to<std::vector>() };
        };

        QueryParams params{};
        for (const auto& rawParam : str | vs::split('&')){
            const auto [r, l] = splitParams(rawParam);
            params[r] = l;
        }

        return params;
    }

    std::optional<QueryParams> QueryParams::ParseDecodified(std::string_view str) {
        return HttpUrl::TryDecode(str).transform(Parse);
    }

    bool QueryParams::Exists(QueryKeyRef key) const { return _elements.contains(key); }

    bool QueryParams::ValExists(QueryKeyRef key, QueryValueRef val) const {
        const auto&& it{ _elements.find(key) };
        if (it == _elements.end())
            return false;

        const auto&& it2{ std::ranges::find(it->second, val) };

        return it2 != it->second.end();
    }

    void QueryParams::Add(QueryKeyRef key, QueryValueRef val) {
        _elements[key].push_back(val);
    }

    bool QueryParams::Remove(QueryKeyRef key, QueryValueRef val) {
        auto&& it = _elements.find(key);
        if (it == _elements.end())
            return false;

        auto& vec = it->second;
        const auto oldSize{ vec.size() };
        const auto newEnd = std::ranges::remove(vec, val).begin();
        vec.erase(newEnd, vec.end());

        return oldSize != vec.size();
    }

    bool QueryParams::RemoveKey(QueryKeyRef key) {
        auto&& it = _elements.find(key);
        if (it == _elements.end())
            return false;

        _elements.erase(it);
        return true;
    }

    bool QueryParams::SetIfNull(QueryKeyRef key, QueryValueRef value) {
        if (Exists(key))
            return false;

        _elements[key].push_back(value);
        return true;
    }

    std::optional<std::reference_wrapper<QueryParams::QueryValues>> QueryParams::Get(QueryKeyRef key) {
        if (Exists(key))
            return _elements[key];

        return std::nullopt;
    }

    QueryParams::IterType QueryParams::begin() { return _elements.begin(); }

    QueryParams::IterType QueryParams::end() { return _elements.end(); }

    QueryParams::CIterType QueryParams::begin() const { return _elements.cbegin(); }

    QueryParams::CIterType QueryParams::end() const { return _elements.cend(); }

    QueryParams::RIterType QueryParams::rbegin() { return _elements.rbegin(); }

    QueryParams::RIterType QueryParams::rend() { return _elements.rend(); }

    QueryParams::CRIterType QueryParams::rbegin() const { return _elements.crbegin(); }

    QueryParams::CRIterType QueryParams::rend() const { return _elements.crend(); }

    void QueryParams::Clear() { _elements.clear(); }

    size_t QueryParams::Size() const { return _elements.size(); }

    bool QueryParams::Empty() const { return _elements.size() == 0; }

    QueryParams::QueryValues& QueryParams::operator[](QueryKeyRef key) { return _elements[key]; }

    bool QueryParams::operator==(const QueryParams& other) const = default;
}

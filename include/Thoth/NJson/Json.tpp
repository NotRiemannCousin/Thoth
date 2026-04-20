#pragma once
#include <format>
#include <ranges>

#include <Thoth/Http/RequestError.hpp>
#include <Thoth/Utils/LastMatchVariant.hpp>
#include <Thoth/Utils/Overloads.hpp>
#include <Thoth/NJson/JsonObject.hpp>

// TODO: FUTURE: Encapsulate loops for array and objects in JsonUtil? also a nested find (e.g. (json, "map", "continents", "countries"))?

// NOLINTNEXTLINE(*)
namespace Thoth::NJson {

#pragma region Construction and Assignment

    template<class T>
        requires std::floating_point<T> || std::integral<T> && (!std::same_as<T, bool>)
    Json::Json(T other) {
        _value = static_cast<Number>(other);
    }

    template<class T>
    requires std::constructible_from<std::string, T>
    Json::Json(T&& other) {
        _value = String::FromOwned(std::string{ std::forward<T>(other) });
    }

    template<class T>
    requires std::floating_point<T> || std::integral<T> && (!std::same_as<T, bool>)
    Json& Json::operator=(T other) {
        _value = static_cast<Number>(other);
        return *this;
    }

    template<class T>
        requires std::constructible_from<std::string, T>
    Json& Json::operator=(T&& other) {
        _value = String::FromOwned(std::forward<T>(other));
        return *this;
    }

#pragma endregion

    template<class T>
    bool Json::IsOfType(const Json& val) {
        return std::holds_alternative<T>(val._value);
    }

    template<class T>
    bool Json::IsOf() const {
        return std::holds_alternative<T>(_value);
    }

    template<class T>
    T& Json::AsType(Json& val) {
        return std::get<T>(val._value);
    }


#pragma region Macro Definitions

#pragma push_macro("CHECK_TYPE")
#pragma push_macro("RETURN_TYPE")
#pragma push_macro("RET_CPY")
#pragma push_macro("RET_MUT")
#pragma push_macro("RET_MOV")
#pragma push_macro("RET_REF")
#pragma push_macro("ERROR")
#pragma push_macro("MOV_FROM")
#undef CHECK_TYPE
#undef RETURN_TYPE
#undef RET_CPY
#undef RET_MUT
#undef RET_MOV
#undef RET_REF
#undef ERROR
#undef MOV_FROM

#define CHECK_TYPE(happyPath, badPath) if (IsOf<T>()) return happyPath; return badPath;

#define RETURN_TYPE(RET) std::optional<RET>
#define RET_CPY RETURN_TYPE(T)
#define RET_MUT RETURN_TYPE(T*)
#define RET_MOV RETURN_TYPE(T)
#define RET_REF RETURN_TYPE(const T*)

#define MOV_FROM(FROM) std::move(*this).template FROM
    // even with std::optional<T*>, will I still need to use T* because of expected? aff, hard life

#define ERROR \
    std::unexpected{ Http::RequestError{ Http::JsonWrongTypeError{ \
        Http::JsonWrongTypeError::IndexOf<T>, _value.index() } } }

#pragma endregion

#pragma region As

    template<class T, class Self>
    decltype(auto) Json::As(this Self&& self) {
        return std::get<T>(std::forward_like<Self>(self._value));
    }

    template<class T> T        Json::AsCpy() const& { return As<T>();           }
    template<class T> T&       Json::AsMut() &      { return As<T>();           }
    template<class T> T&&      Json::AsMov() &&     { return MOV_FROM(As<T>()); }
    template<class T> const T& Json::AsRef() const& { return As<T>();           }

#pragma endregion

#pragma region Ensure

    template<class T> RET_MUT Json::Ensure() &      { CHECK_TYPE(&As<T>(), std::nullopt);          }
    template<class T> RET_MOV Json::Ensure() &&     { CHECK_TYPE(MOV_FROM(As<T>()), std::nullopt); }
    template<class T> RET_REF Json::Ensure() const& { CHECK_TYPE(&As<T>(), std::nullopt);          }


    template<class T> RET_CPY Json::EnsureCpy() const& { CHECK_TYPE(&As<T>(), std::nullopt); }
    template<class T> RET_MUT Json::EnsureMut() &      { return Ensure<T>();                 }
    template<class T> RET_MOV Json::EnsureMov() &&     { return MOV_FROM(Ensure<T>());       }
    template<class T> RET_REF Json::EnsureRef() const& { return Ensure<T>();                 }

#pragma endregion

#pragma region EnsureOrError

#undef RETURN_TYPE
#define RETURN_TYPE(RET) std::expected<RET, Http::RequestError>

    template<class T> RET_MUT Json::EnsureOrError() &      { CHECK_TYPE(&As<T>(), ERROR);          }
    template<class T> RET_MOV Json::EnsureOrError() &&     { CHECK_TYPE(MOV_FROM(As<T>()), ERROR); }
    template<class T> RET_REF Json::EnsureOrError() const& { CHECK_TYPE(&As<T>(), ERROR);          }


    template<class T> RET_CPY Json::EnsureCpyOrError() const& { CHECK_TYPE(As<T>(), ERROR);          }
    template<class T> RET_MUT Json::EnsureMutOrError() &      { return EnsureOrError<T>();           }
    template<class T> RET_MOV Json::EnsureMovOrError() &&     { return MOV_FROM(EnsureOrError<T>()); }
    template<class T> RET_REF Json::EnsureRefOrError() const& { return EnsureOrError<T>();           }

#pragma endregion

#pragma region Macro Cleanup

#pragma pop_macro("CHECK_TYPE")
#pragma pop_macro("RETURN_TYPE")
#pragma pop_macro("RET_CPY")
#pragma pop_macro("RET_MUT")
#pragma pop_macro("RET_MOV")
#pragma pop_macro("RET_REF")
#pragma pop_macro("ERROR")
#pragma pop_macro("MOV_FROM")

#pragma endregion



    template<class Callable>
    constexpr decltype(auto) Json::Visit(Callable&& callable) {
        return std::visit(std::forward<Callable>(callable), _value);
    }

    template<class Callable>
    constexpr decltype(auto) Json::Visit(Callable&& callable) const {
        return std::visit(std::forward<Callable>(callable), _value);
    }


#pragma region Search Functions
#pragma push_macro("RETURN_IF_MATCH")
#undef RETURN_IF_MATCH
#define RETURN_IF_MATCH(declaration, expr) {                   \
    if (IsOf<Array>()) {                                       \
        for (declaration : As<Array>())                        \
            if (std::invoke(pred, obj))                        \
                return expr;                                   \
    }                                                          \
    if (IsOf<Object>()) {                                      \
        for (declaration : *As<Object>() | std::views::values) \
            if (std::invoke(pred, obj))                        \
                return expr;                                   \
    }                                                          \
}


    template <class Pred> requires std::predicate<Pred, Json>
    OptRefValWrapper Json::Search(Pred&& pred) {
        RETURN_IF_MATCH(auto &obj, &obj);
        return std::nullopt;
    }
    template <class Pred> requires std::predicate<Pred, Json>
    [[nodiscard]] OptCRefValWrapper Json::Search(Pred&& pred) const {
        RETURN_IF_MATCH(const auto &obj, &obj);
        return std::nullopt;
    }
    template<class Pred> requires std::predicate<Pred, Json>
    OptValWrapper Json::SearchCopy(Pred &&pred) const {
        RETURN_IF_MATCH(const auto &obj, obj);
        return std::nullopt;
    }
    template<class Pred> requires std::predicate<Pred, Json>
    OptValWrapper Json::SearchAndMove(Pred &&pred) && {
        RETURN_IF_MATCH(auto &obj, std::move(obj));
        return std::nullopt;
    }


    template<class Pred> requires std::predicate<Pred, Json>
    ValWrapper Json::SearchCopyOrNull(Pred &&pred) const {
        RETURN_IF_MATCH(auto &obj, obj);
        return NullJ;
    }
    template<class Pred> requires std::predicate<Pred, Json>
    ValWrapper Json::SearchAndMoveOrNull(Pred &&pred) {
        RETURN_IF_MATCH(auto &obj, std::move(obj));
        return NullJ;
    }


    template<class Pred> requires std::predicate<Pred, Json>
    ExpRefValWrapper Json::SearchOrError(Pred &&pred) {
        RETURN_IF_MATCH(auto &obj, &obj);
        return std::unexpected{ Http::RequestError{ Http::JsonSearchError{} } };
    }
    template<class Pred> requires std::predicate<Pred, Json>
    ExpCRefValWrapper Json::SearchOrError(Pred &&pred) const {
        RETURN_IF_MATCH(auto &obj, &obj);
        return std::unexpected{ Http::RequestError{ Http::JsonSearchError{} } };
    }
    template<class Pred> requires std::predicate<Pred, Json>
    ExpValWrapper Json::SearchCopyOrError(Pred &&pred) const {
        RETURN_IF_MATCH(auto &obj, obj);
        return std::unexpected{ Http::RequestError{ Http::JsonSearchError{} } };
    }
    template<class Pred> requires std::predicate<Pred, Json>
    ExpValWrapper Json::SearchAndMoveOrError(Pred &&pred) && {
        RETURN_IF_MATCH(auto &obj, std::move(obj));
        return std::unexpected{ Http::RequestError{ Http::JsonSearchError{} } };
    }

#pragma pop_macro("RETURN_IF_MATCH")
#pragma endregion

    namespace detail {
        using OutIt =  std::format_context::iterator;

        template<class OutIt>
        void FormatJsonVal(const Json& val, bool pretty, const std::string& indent,
            size_t indentDepth, OutIt& it, std::string& tempOutBuffer);


        template<class OutIt>
        void EscapeJsonString(const std::string_view& str, const OutIt& it, std::string& tempOutBuffer) {
            tempOutBuffer = '"';
            tempOutBuffer.reserve(str.size());

            for (const unsigned char c : str) {
                switch (c) {
                    case '"':  tempOutBuffer += R"(\")"; break;
                    case '\\': tempOutBuffer += R"(\\)"; break;
                    case '\b': tempOutBuffer += R"(\b)"; break;
                    case '\f': tempOutBuffer += R"(\f)"; break;
                    case '\n': tempOutBuffer += R"(\n)"; break;
                    case '\r': tempOutBuffer += R"(\r)"; break;
                    case '\t': tempOutBuffer += R"(\t)"; break;
                    default:
                        if (c < 32)
                            tempOutBuffer += std::format("\\u{:04x}", static_cast<unsigned char>(c));
                        else
                            tempOutBuffer += static_cast<char>(c);
                }
            }

            tempOutBuffer += '"';
            std::format_to(it, "{}", tempOutBuffer);
        }

        template<class OutIt>
        void FormatJsonObj(const JsonObject& json, bool pretty, const std::string& indent,
            size_t indentDepth, OutIt& it, std::string& tempOutBuffer) {

            std::format_to(it, "{{");

            indentDepth++;
            bool first = true;
            for (const auto& [key, val] : json) {
                if (!first) std::format_to(it, ",");
                first = false;

                if (pretty) {
                    std::format_to(it, "\n");
                    for (int i{}; i < indentDepth; i++)
                        std::format_to(it, "{}", indent);
                }


                EscapeJsonString(key, it, tempOutBuffer);
                std::format_to(it, ":");


                if (pretty) std::format_to(it, " ");

                FormatJsonVal(val, pretty, indent, indentDepth + 1, it, tempOutBuffer);
            }
            indentDepth--;

            if (pretty && !json.Empty()) {
                std::format_to(it, "\n");
                for (int i{}; i < indentDepth; i++)
                    std::format_to(it, "{}", indent);
            }

            std::format_to(it, "}}");
        }

        template<class OutIt>
        void FormatJsonArr(const Array& val, bool pretty, const std::string& indent,
                size_t indentDepth, OutIt& it, std::string& tempOutBuffer) {
            std::format_to(it, "[");
            bool first = true;

            indentDepth++;
            for (const auto& elem : val) {
                if (!first) std::format_to(it, ",");
                first = false;

                if (pretty) {
                    std::format_to(it, "\n");
                    for (int i{}; i < indentDepth; i++)
                        std::format_to(it, "{}", indent);
                }

                FormatJsonVal(elem, pretty, indent, indentDepth, it,  tempOutBuffer);
            }

            if (pretty && !val.empty()) {
                std::format_to(it, "\n");
                for (int i{}; i < indentDepth; i++)
                    std::format_to(it, "{}", indent);
            }
            indentDepth--;


            if (pretty) {
                std::format_to(it, "\n");
                for (int i{}; i < indentDepth; i++)
                    std::format_to(it, "{}", indent);
            }

            std::format_to(it, "]");
        }

        template<class OutIt>
        void FormatJsonVal(const Json& val, bool pretty, const std::string& indent,
            const size_t indentDepth, OutIt& it, std::string& tempOutBuffer) {
            std::visit(Utils::Overloaded{
                [&](const String& str){
                    str.Visit(Utils::Overloaded{
                        [&](const String::RefType ref) {
                            std::format_to(it, R"("{}")", ref);
                        },
                        [&](const String::OwnType& own) {
                            EscapeJsonString(own, it, tempOutBuffer);
                        }
                    });
                },
                [&](const Number  num){ std::format_to(it, "{}", num); },
                [&](const Bool    bln){ std::format_to(it, "{}", bln); },
                [&](const Object& obj){ FormatJsonObj(*obj, pretty, indent, indentDepth, it, tempOutBuffer); },
                [&](const Array&  arr){ FormatJsonArr(arr, pretty, indent, indentDepth, it, tempOutBuffer); },
                [&](const Null&      ){ std::format_to(it, "null"); }
            }, static_cast<const Json::Value&>(val));
        }
    }
}


template<>
struct std::formatter<Thoth::NJson::Json> {
    bool pretty{};
    size_t indentLevel{};

    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it != '}') {
            ++it;
            pretty = true;
            auto [_, err]{ std::from_chars(
                &*ctx.begin(),
                &*ctx.begin() + std::distance(ctx.begin(), ctx.end()),
                indentLevel
            ) };

            if (err != std::errc{})
                throw std::format_error("Invalid format specifier for JsonVal");
        }
        return it;
    }

    template<class FormatContext>
    auto format(const Thoth::NJson::Json& val, FormatContext& ctx) const {
        auto it = ctx.out();
        std::string tempOutBuffer;

        Thoth::NJson::detail::FormatJsonVal(val, pretty, std::string(indentLevel, ' '),
            0, it, tempOutBuffer);
        return it;
    }
};

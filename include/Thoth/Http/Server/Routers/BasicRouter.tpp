#pragma once

#include <algorithm>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/Response/ResponseEntry.hpp>
#include <Thoth/Http/Response/StatusCodeEnum.hpp>
#include <Thoth/Http/Server/Definitions.hpp>
#include <Thoth/Http/_base/Payload/_base/_pch.hpp>
#include <Thoth/Utils/Traits.hpp>

namespace Thoth::Http {

    namespace details_ {

#pragma region details_::BasicRouteCallbackTraits

        template<class Ret, class Req, class... Args>
        struct BasicRouteCallbackSignature : std::false_type {};

        template<MethodConcept MethodT, DefaultIncomingBodyConcept RequestBodyT, OutgoingBodyConcept ResponseBodyT, class... Args>
        struct BasicRouteCallbackSignature<Response<MethodT, ResponseBodyT>, Request<MethodT, RequestBodyT>, Args...>
            : std::true_type {
            using Method       = MethodT;
            using RequestBody  = RequestBodyT;
            using ResponseBody = ResponseBodyT;
            using Result       = Response<MethodT, ResponseBodyT>;
            using Signature    = Result(Request<MethodT, RequestBodyT>&, Args...);
            using Params       = std::tuple<Args...>;
        };

        template<MethodConcept MethodT, DefaultIncomingBodyConcept RequestBodyT, OutgoingBodyConcept ResponseBodyT, class... Args>
        struct BasicRouteCallbackSignature<Response<MethodT, ResponseBodyT>, const Request<MethodT, RequestBodyT>, Args...>
            : BasicRouteCallbackSignature<Response<MethodT, ResponseBodyT>, Request<MethodT, RequestBodyT>, Args...> {};

        template<MethodConcept MethodT, DefaultIncomingBodyConcept RequestBodyT, OutgoingBodyConcept ResponseBodyT, class... Args>
        struct BasicRouteCallbackSignature<ResponseEntry<MethodT, ResponseBodyT>, Request<MethodT, RequestBodyT>, Args...>
            : BasicRouteCallbackSignature<Response<MethodT, ResponseBodyT>, Request<MethodT, RequestBodyT>, Args...> {};

        template<MethodConcept MethodT, DefaultIncomingBodyConcept RequestBodyT, OutgoingBodyConcept ResponseBodyT, class... Args>
        struct BasicRouteCallbackSignature<ResponseEntry<MethodT, ResponseBodyT>, const Request<MethodT, RequestBodyT>, Args...>
            : BasicRouteCallbackSignature<Response<MethodT, ResponseBodyT>, Request<MethodT, RequestBodyT>, Args...> {};


        template<class T, class = void>
        struct BasicRouteCallbackTraits : std::false_type {};

        template<class Ret, class Req, class... Args>
        struct BasicRouteCallbackTraits<Ret (*)(Req&, Args...)> : BasicRouteCallbackSignature<Ret, Req, Args...> {};

        template<class ClassType, class Ret, class Req, class... Args>
        struct BasicRouteCallbackTraits<Ret (ClassType::*)(Req&, Args...) const> : BasicRouteCallbackSignature<Ret, Req, Args...> {};

        template<class ClassType, class Ret, class Req, class... Args>
        struct BasicRouteCallbackTraits<Ret (ClassType::*)(Req&, Args...)>
            : BasicRouteCallbackTraits<Ret (ClassType::*)(Req&, Args...) const> {};

        template<class T>
        struct BasicRouteCallbackTraits<T, std::void_t<decltype(&T::operator())>>
            : BasicRouteCallbackTraits<decltype(&T::operator())> {};

        template<class ClassType, class Ret, class Req, class... Args>
        struct BasicRouteCallbackTraits<Ret (ClassType::*)(Req&, Args...) const noexcept>
            : BasicRouteCallbackSignature<Ret, Req, Args...> {};

        template<class ClassType, class Ret, class Req, class... Args>
        struct BasicRouteCallbackTraits<Ret (ClassType::*)(Req&, Args...) noexcept>
            : BasicRouteCallbackSignature<Ret, Req, Args...> {};

#pragma endregion

        template<class Socket, class... ExpectedMiddlewares>
            requires (BasicRouterMiddlewareConcept<ExpectedMiddlewares, Socket> && ...)
        constexpr bool AreMiddlewaresNonOverlapping() {
            return Utils::IsUnique<
                typename BasicRouterMiddlewareTraits<ExpectedMiddlewares, Socket>::RequiredResult...,
                typename BasicRouterMiddlewareTraits<ExpectedMiddlewares, Socket>::OptionalResult...
            >();
        }

        template<class Socket, class Param, class... AllMws>
        constexpr bool MatchesAnyMiddleware{
            ((std::same_as<Param, typename BasicRouterMiddlewareTraits<AllMws, Socket>::RequiredResult> ||
              std::same_as<Param, typename BasicRouterMiddlewareTraits<AllMws, Socket>::OptionalResult>) || ...)
        };

        template<class Socket, class Param, class... AllMws>
        struct NormalizeMiddlewareParam {
            using Type = Param;
        };

        template<class Socket, class T, class... AllMws>
            requires (std::same_as<std::optional<T>, typename BasicRouterMiddlewareTraits<AllMws, Socket>::OptionalResult> || ...)
        struct NormalizeMiddlewareParam<Socket, std::optional<T>, AllMws...> {
            using Type = T;
        };

        template<class Socket, class ParamsTuple, class... AllMws>
        constexpr void ValidateCallbackMiddlewares() {
            [&]<class... Params>(std::type_identity<std::tuple<Params...>>) {
                static_assert(
                    (MatchesAnyMiddleware<Socket, std::remove_cvref_t<Params>, AllMws...> && ...),
                    "Route callback contains an argument that does not match any middleware in MiddlewarePack "
                    "(each argument must be either T or std::optional<T>, where T is a middleware's value_type)."
                );

                static_assert(
                    Utils::IsUnique<std::remove_cvref_t<Params>...>(),
                    "Route callback middleware arguments must all be different (duplicated argument types are not allowed)."
                );

                static_assert(
                    Utils::IsUnique<typename NormalizeMiddlewareParam<Socket, std::remove_cvref_t<Params>, AllMws...>::Type...>(),
                    "Route callback middleware arguments cannot intersect (cannot request both T and std::optional<T> "
                    "for the same middleware)."
                );
            }(std::type_identity<ParamsTuple>{});
        }

        template<
            class Socket,
            class ParamsTuple,
            std::size_t MIdx = 0,
            std::size_t PIdx = 0,
            class... AllMws,
            class... MatchedMws
        >
        constexpr auto FilterMatchingMiddlewares(const MiddlewarePack<AllMws...>& allMws, MiddlewarePack<MatchedMws...> matched = {}) {
            if constexpr (PIdx == std::tuple_size_v<ParamsTuple>) {
                return matched;
            }
            else if constexpr (MIdx == sizeof...(AllMws)) {
                static_assert(
                    PIdx == std::tuple_size_v<ParamsTuple>,
                    "Route callback middleware arguments must be declared in the exact same order as their middlewares "
                    "inside MiddlewarePack (either T or std::optional<T>)."
                );
                return matched;
            } else {
                using CurrentMw  = std::tuple_element_t<MIdx, std::tuple<AllMws...>>;
                using MwTraits   = BasicRouterMiddlewareTraits<CurrentMw, Socket>;
                using CleanParam = std::remove_cvref_t<std::tuple_element_t<PIdx, ParamsTuple>>;

                constexpr bool match{
                    std::same_as<CleanParam, typename MwTraits::RequiredResult> ||
                    std::same_as<CleanParam, typename MwTraits::OptionalResult>
            };

                if constexpr (match) {
                    auto nextMatched{ std::tuple_cat(matched, std::make_tuple(std::get<MIdx>(allMws))) };
                    return FilterMatchingMiddlewares<Socket, ParamsTuple, MIdx + 1, PIdx + 1>(allMws, nextMatched);
                } else {
                    return FilterMatchingMiddlewares<Socket, ParamsTuple, MIdx + 1, PIdx>(allMws, matched);
                }
            }
        }

    }


    template<CommonSocketConcept Socket>
    template<class F, class... Middlwares>
    bool BasicRouter<Socket>::MatchOne(
        Exchange<Socket>& sock, std::optional<ResolvedPair>& out, F& callback, MiddlewarePack<Middlwares...> mws) {

        using Traits = details_::BasicRouteCallbackTraits<std::decay_t<F>>;

        details_::ValidateCallbackMiddlewares<Socket, typename Traits::Params, Middlwares...>();
        auto activeMws{ details_::FilterMatchingMiddlewares<Socket, typename Traits::Params>(mws) };

        if (Traits::Method::MethodName() != sock.method)
            return false;

        out.emplace(RouteImpl(
            sock,
            sock.template ReadRequest<typename Traits::Method, typename Traits::RequestBody>(),
            callback,
            std::move(activeMws)
        ));

        return true;
    }


    template<CommonSocketConcept Socket>
    template<class F>
    void BasicRouter<Socket>::Route(std::string path, F&& callback) {
        Match(std::move(path), Use(), std::forward<F>(callback));
    }

    template<CommonSocketConcept Socket>
    template<class F, class ... Middlwares>
    void BasicRouter<Socket>::Route(std::string path, MiddlewarePack<Middlwares...> mws, F&& callback) {
        Match(std::move(path), mws, std::forward<F>(callback));
    }

    template<CommonSocketConcept Socket>
    template<class... Fs>
        requires (sizeof...(Fs) > 0)
    void BasicRouter<Socket>::Match(std::string path, Fs... callbacks) {
        return Match(std::move(path), Use(), std::forward<Fs>(callbacks)...);
    }

    template<CommonSocketConcept Socket>
    template<class ... Middlewares, class ... Fs> requires (sizeof...(Fs) > 0)
    void BasicRouter<Socket>::Match(std::string path, MiddlewarePack<Middlewares...> mws, Fs... callbacks) {
        static_assert(
            (BasicRouterMiddlewareConcept<Middlewares, Socket> && ...),
            "Invalid middleware callable. Every middleware must accept `const Exchange<Socket>&` and return either "
            "`std::optional<T>` or `std::expected<T, E>` (where `E` satisfies `IntoResponsePayloadConcept<AnyBodyOut, E>`).");

        static_assert(
            (details_::BasicRouteCallbackTraits<std::decay_t<Fs>>{} && ...),
            "Route callback method mismatch, every callback needs to match a signature similar to "
            "`Response<Method, OutcomingBody>(Request<Method, DefaultIncomingBody>&)` (or returning "
            "`ResponseEntry<Method, OutcomingBody>`). The Method is the same for both, the request and response.");

        static_assert(
            Utils::IsUnique<typename details_::BasicRouteCallbackTraits<std::decay_t<Fs>>::Method...>(),
            "Duplicated request body type. Every route callback must match a different callback.");

        static_assert(
            details_::AreMiddlewaresNonOverlapping<Socket, Middlewares...>(),
            "Duplicated or overlapping middleware types. Every middleware in MiddlewarePack must produce a distinct "
            "type T without intersecting with another middleware's T or std::optional<T>.");

        m_routes[std::move(path)] = [callbacks..., mws](Exchange<Socket>& sock) mutable -> ResolvedPair {
            std::optional<ResolvedPair> out{};

            (MatchOne(sock, out, callbacks, mws) || ...);

            if (!out) throw StatusCodeEnum::MethodNotAllowed;

            return std::move(*out);
        };
    }


    template<CommonSocketConcept Socket>
    auto BasicRouter<Socket>::Resolve(Exchange<Socket>& exchange) -> ResolvedPair {
        auto callback{ m_routes.find(exchange.head.url.GetPathOrSep()) };

        try {
            if (callback != m_routes.end())
                co_return co_await callback->second(exchange);
        } catch (StatusCodeEnum code) {
            co_return std::pair{ ResponseHead{ exchange.head.version, code }, AnyBodyOut{ std::string{} } };
        }

        co_return std::pair{
            ResponseHead{
                exchange.head.version, StatusCodeEnum::NotFound,
                std::move(exchange.head.headers)
            },
            AnyBodyOut{ std::string{} }
        };
    }

    template<CommonSocketConcept Socket>
    template<class ReqTask, class F, class... Middlewares>
    auto BasicRouter<Socket>::RouteImpl(
        const Exchange<Socket>& sock, ReqTask request, F&& callback, MiddlewarePack<Middlewares...> mds) -> ResolvedPair {
        using Traits      = details_::BasicRouteCallbackTraits<std::decay_t<F>>;
        using ParamsTuple = typename Traits::Params;

        auto args{ []<class... Params>(std::type_identity<std::tuple<Params...>>) {
            return std::tuple<std::optional<std::remove_cvref_t<Params>>...>{};
        }(std::type_identity<ParamsTuple>{}) };

        std::optional<std::pair<ResponseHead, AnyBodyOut>> errorResponse{};

        auto runOne{ [&]<std::size_t I>(std::integral_constant<std::size_t, I>) -> bool {
            using CurrentMw  = std::tuple_element_t<I, MiddlewarePack<Middlewares...>>;
            using MwTraits   = BasicRouterMiddlewareTraits<CurrentMw, Socket>;
            using CleanParam = std::remove_cvref_t<std::tuple_element_t<I, ParamsTuple>>;
            using MwResult   = std::remove_cvref_t<typename MwTraits::Result>;

            constexpr bool isRequired{ std::same_as<CleanParam, typename MwTraits::RequiredResult> };

            auto mwRes{ std::invoke(std::get<I>(mds), sock) };
            if (mwRes.has_value()) {
                std::get<I>(args).emplace(*std::move(mwRes));
                return true;
            }

            if constexpr (!isRequired) {
                std::get<I>(args).emplace(std::nullopt);
                return true;
            } else {
                if constexpr (Utils::IsOptional<MwResult>) {
                    errorResponse.emplace(
                        ResponseHead{ sock.head.version, StatusCodeEnum::BadRequest },
                        AnyBodyOut{ std::string{} }
                    );
                } else {
                    using ErrorType = typename MwResult::error_type;
                    errorResponse.emplace(
                        static_cast<IntoResponsePayload<AnyBodyOut>>(
                            IntoPayloadFor<AnyBodyOut, ErrorType>{ std::move(mwRes).error() }
                        )
                    );
                }
                return false;
            }
        } };

        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            (runOne(std::integral_constant<std::size_t, Is>{}) && ...);
        }(std::index_sequence_for<Middlewares...>{});

        if (errorResponse)
            co_return std::move(*errorResponse);

        auto typedRequest{ co_await std::move(request) };

        auto forwardArg{ []<class Param, class Opt>(Opt& opt) -> decltype(auto) {
            if constexpr (std::is_lvalue_reference_v<Param>)
                return static_cast<Param>(*opt);
            else
                return std::move(*opt);
        } };

        typename Traits::Result res = [&]<class... Params, std::size_t... Is>(
            std::type_identity<std::tuple<Params...>>, std::index_sequence<Is...>) -> typename Traits::Result {
            return callback(typedRequest, forwardArg.template operator()<Params>(std::get<Is>(args))...);
        }(std::type_identity<ParamsTuple>{}, std::index_sequence_for<Middlewares...>{});

        co_return std::pair<ResponseHead, AnyBodyOut>{
            static_cast<ResponseHead&&>(res),
            std::move(res.body)
        };
    }
}
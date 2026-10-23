#pragma once
#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <utility>

#include <exec/task.hpp>

#include <Thoth/Dsa/LinearMap.hpp>
#include <Thoth/Http/Server/Definitions.hpp>
#include <Thoth/Http/Server/Router.hpp>

namespace Thoth::Http {
    namespace details_ {
        template<class T>
        constexpr bool BasicRouterReturnHelper{ false };
        template<class T>
        constexpr bool BasicRouterReturnHelper<std::optional<T>>{ true };
        template<class T, class E>
            requires IntoResponsePayloadConcept<AnyBodyOut, E>
        constexpr bool BasicRouterReturnHelper<std::expected<T, E>>{ true };
    }

    //! @brief Defines a Middleware function.
    template<class T, class Socket>
    concept BasicRouterMiddlewareConcept = std::invocable<T, const Exchange<Socket>&> &&
            details_::BasicRouterReturnHelper<std::invoke_result_t<T, const Exchange<Socket>&>>;


    template<class, class, class=void>
    struct BasicRouterMiddlewareTraits : std::false_type {};

    template<class T, class Socket>
        requires BasicRouterMiddlewareConcept<T, Socket>
    struct BasicRouterMiddlewareTraits<T, Socket> : std::true_type {
        using Result = std::invoke_result_t<T, const Exchange<Socket>&>;

        using RequiredResult = Result::value_type;
        using OptionalResult = std::optional<RequiredResult>;

        using ResultPair = std::pair<RequiredResult, OptionalResult>;
    };


    template<class... Mws>
    using MiddlewarePack = std::tuple<Mws...>;

    template<class... Mws>
    auto Use(Mws&&... mws) {
        return MiddlewarePack<std::decay_t<Mws>...>{ std::make_tuple(std::forward<Mws>(mws)...) };
    }


    //! @brief A simple router.
    template<CommonSocketConcept Socket>
    struct BasicRouter {
        using ResolvedPair = exec::task<std::pair<ResponseHead, AnyBodyOut>>;


        template<class F>
        void Route(std::string path, F&& callback);

        template<class F, class... Middlwares>
        void Route(std::string path, MiddlewarePack<Middlwares...> mws, F&& callback);

        template<class... Fs>
            requires (sizeof...(Fs) > 0)
        void Match(std::string path, Fs... callbacks);

        template<class... Middlewares, class... Fs>
            requires (sizeof...(Fs) > 0)
        void Match(std::string path, MiddlewarePack<Middlewares...> mws, Fs... callbacks);
        // calbacks are like [](Request<Method, DefaultIncomingBody>&& req) -> Response<Method, OutcomingBody> {}

        auto Resolve(Exchange<Socket>& exchange) -> ResolvedPair;

    private:
        //! @brief One callback's dispatch, the fold's body of @ref Match: the wire method picks the callback, the
        //! picked one leaves its @ref ResolvedPair in @p out. False keeps the fold going, true ends it.
        template<class F, class... Middlwares>
        static bool MatchOne(Exchange<Socket>& sock, std::optional<ResolvedPair>& out, F& callback, MiddlewarePack<Middlwares...> mws);

        template<class ReqTask, class F, class... Middlewares>
        static auto RouteImpl(const Exchange<Socket>& sock, ReqTask request, F&& callback, MiddlewarePack<Middlewares...> mds) -> ResolvedPair;
        using CallbackInvoker = std::move_only_function<ResolvedPair(Exchange<Socket>&)>;

        Dsa::LinearMap<std::string, CallbackInvoker> m_routes;
    };
}

#include <Thoth/Http/Server/Routers/BasicRouter.tpp>
// POST /mortem, a tiny forum. Posts with tags, replies that quote other posts, everything in memory.
//
// Features:
//  - HTML rendered on the server by Html::Component (see Html.hpp): every visual piece is a `.comp.html` file under
//    public/components, formatted with std::vformat;
//  - a site and an API side by side: the pages are `GET`s that always answer 200 and tell what went wrong in a
//    `<dialog>`, everything that changes something is a JSON endpoint under "/api" that answers with the honest status
//    code, and `app.js` is the glue between both (see the table below);
//  - CRUD on "/api/posts": POST, PUT (both with a JSON body) and DELETE (no body) share one path, and each callback
//    declares its own Method and Body type;
//  - middlewares (`Use(auth, targetId)`): who is logged in and which `?id=` is targeted are resolved from the request
//    head, before the router reads a single byte of the body;
//  - cookies, for a toy login (Accounts.hpp, read the warning there, Thoth has no auth, this one is NOT for
//    real use);
//  - the forum state on singletons (Board, Accounts): the handlers reach them through Instance() and pass
//    nothing around. A real deployment would put this state behind another service or a database, keeping it in
//    process is what makes the example about the networking.
//
// All routes lies in the main function. The files are read from the `public` directory, the build copies it next to the
// executable, so run it from there or pass the directory as the first argument. The second argument is the endpoint
// string:
//     PostMortemExample [public-dir="public"] [endpoint="127.0.0.1:8080"]
// Then, in the terminal:
//  - Enter stops the server gracefully;
//  - `reload` + Enter reloads public/components without stopping, so edits to the `.comp.html` files show up on the
//    very next request.
//
// Thoth's BasicRouter fixes the HTTP method and the body type to the Request (even thoght the Request and Response body
// types can differ, the methods must match). The routes are:
//
// | Route                       | Request body              | Answer                                   |
// |-----------------------------|---------------------------|------------------------------------------|
// | `GET    /`                  | none (`?tag=`, `?reply=`) | the forum, HTML (always 200)             |
// | `GET /style.css`, `/app.js` | none                      | the static files                         |
// | `POST   /api/login`         | JSON (`std::string`)      | 204 + session cookie                     |
// | `POST   /api/logout`        | none                      | 204, cookie cleared                      |
// | `POST   /api/posts`         | JSON (`std::string`)      | 201, `{ "id", "href" }` of the new post  |
// | `PUT    /api/posts?id=N`    | JSON (`std::string`)      | the updated post, JSON                   |
// | `DELETE /api/posts?id=N`    | nothing (`EmptyBody`)     | 204 No Content                           |
// | `POST   /api/replies`       | JSON (`std::string`)      | 201, `{ "id", "href" }` of the new reply |
// | `DELETE /api/replies?id=N`  | nothing (`EmptyBody`)     | 204 No Content                           |

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <format>
#include <iostream>
#include <optional>
#include <print>
#include <unordered_set>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include <stdexec/execution.hpp>

#include <Thoth/Http/Methods/DeleteMethod.hpp>
#include <Thoth/Http/Methods/PutMethod.hpp>
#include <Thoth/Http/NHeaders/Headers/MimeType.hpp>
#include <Thoth/Http/NHeaders/Response/Headers/Cookie.hpp>
#include <Thoth/Http/Request/QueryParams.hpp>
#include <Thoth/Http/Request/Request.hpp>
#include <Thoth/Http/Response/Response.hpp>
#include <Thoth/Http/Response/ResponseEntry.hpp>
#include <Thoth/Http/Server/Server.hpp>
#include <Thoth/NJson/Json.hpp>
#include <Thoth/ThothError.hpp>
#include <Thoth/Utils/Ranges/FlatMap.hpp>
#include <Thoth/Utils/Expected.hpp>

#include <PostMortem/Accounts.hpp>
#include <PostMortem/Board.hpp>
#include <PostMortem/Html.hpp>


namespace {

    using namespace Thoth::Http;
    using namespace Thoth::NJson;
    namespace Html  = PostMortem::Html;

    namespace vs = std::views;
    namespace rg = std::ranges;

    using PostMortem::Account;
    using PostMortem::Accounts;
    using PostMortem::Board;
    using PostMortem::Post;
    using PostMortem::Reply;

    constexpr std::size_t kMaxTitleBytes  { 400 };
    constexpr std::size_t kMaxContentBytes{ 8000 };
    constexpr std::size_t kMaxTags        { 5 };
    constexpr std::size_t kMaxTagBytes    { 32 };

    const Html::Component kPage        { "components/page.comp.html" };
    const Html::Component kPost        { "components/post.comp.html" };
    const Html::Component kPostActions { "components/post-actions.comp.html" };
    const Html::Component kTagLink     { "components/tag-link.comp.html" };
    const Html::Component kTagOption   { "components/tag-option.comp.html" };
    const Html::Component kReplies     { "components/replies.comp.html" };
    const Html::Component kReply       { "components/reply.comp.html" };
    const Html::Component kReplyActions{ "components/reply-actions.comp.html" };
    const Html::Component kEmptyFeed   { "components/empty-feed.comp.html" };
    const Html::Component kAccount     { "components/account.comp.html" };
    const Html::Component kLogin       { "components/login.comp.html" };
    const Html::Component kComposePost { "components/compose-post.comp.html" };
    const Html::Component kComposeReply{ "components/compose-reply.comp.html" };

#pragma region General Helpers

    template<class Body>
    Response<GetMethod, Body> Page(const NHeaders::MimeType mimeType, Body body) {
        Response<GetMethod, Body> response{};
        response.headers.ContentType() = mimeType;
        response.body = std::move(body);
        return response;
    }

#pragma endregion

#pragma region Parsing Helpers

    //! @brief A required, non-blank text field. Only LF is kept, other clients may send line breaks as CRLF.
    template<size_t maxBytes>
    std::expected<std::string, Thoth::ThothError> RequiredText(const Json* json) {
        auto sanitize{ [](const String* text) -> std::expected<std::string, Thoth::ThothError> {
            std::string str{ Thoth::String::TrimmedStr(text->AsRef()) };
            std::erase(str, '\r');

            const bool hasControlChars{ rg::any_of(str, [](const unsigned char c) {
                if (c == '\n' || c == '\t')
                    return false;
                return c < 0x20 || c == 0x7F;
            }) };

            if (str.empty() || str.size() > maxBytes || hasControlChars)
                return std::unexpected{ StatusCodeEnum::UnprocessableContent };

            return str;
        } };

        return json->EnsureOrError<String>().and_then(sanitize);
    }

    std::optional<std::uint64_t> ParseId(const std::string* raw) {
        std::uint64_t id{};
        const auto [end, ec]{ std::from_chars(raw->data(), raw->data() + raw->size(), id) };
        if (raw->empty() || ec != std::errc{} || end != raw->data() + raw->size())
            return std::nullopt;

        return id;
    }

    //! @brief ["Foo", "#bar", " baz"] -> {"foo", "bar", "baz"}. Lowercase, no repeats, in order, 1 to kMaxTags tags.
    std::expected<std::vector<std::string>, Thoth::ThothError> ParseTags(const Json* raw) {
        using TagList = std::vector<std::string>;

        static constexpr auto parseItem{ [](const String& input) -> std::expected<std::string, Thoth::ThothError> {
            std::string tag{ Thoth::String::Trimmed(input.AsRef(), "# \t\n\v\f\r") };

            rg::transform(tag, tag.begin(), Thoth::String::ToLower);

            if (tag.size() > kMaxTagBytes)
                return std::unexpected{ StatusCodeEnum::UnprocessableContent };

            return std::move(tag);
        } };

        constexpr auto normalize{ [](TagList parsed) -> std::expected<TagList, Thoth::ThothError> {
            TagList tags{};

            for (auto& tag : parsed)
                if (!tag.empty() && !rg::contains(tags, tag))
                    tags.push_back(std::move(tag));

            if (tags.empty() || tags.size() > kMaxTags)
                return std::unexpected{ StatusCodeEnum::UnprocessableContent };

            return tags;
        } };

        constexpr auto parseArray{ [](const Array* arr) {
            return *arr
                | std::views::transform([](const Json& item) { return item.EnsureCpyOrError<String>().and_then(parseItem); })
                | Thoth::Utils::collect;
        } };

        return raw->EnsureRefOrError<Array>().and_then(parseArray).and_then(normalize);
    }

#pragma endregion

#pragma region Rendering

    std::string RenderTagLink(std::string_view selectedTag, const std::vector<std::string>& filter);
    std::string RenderReply(const Reply& reply, const std::optional<Account>& viewer);
    std::string RenderPost(const Post& post, const std::optional<Account>& viewer, const std::vector<std::string>& filter);

    auto H_RenderTagLink(const std::vector<std::string>& filter) {
        return [&](const auto& arg){ return RenderTagLink(arg, filter); };
    }
    auto H_RenderReply(const std::optional<Account>& viewer) {
        return [&](const auto& arg){ return RenderReply(arg, viewer); };
    }
    auto H_RenderPost(const std::optional<Account>& viewer, const std::vector<std::string>& filter) {
        return [&](const auto& arg){ return RenderPost(arg, viewer, filter); };
    }

    //! @brief One tag chip. `active` marks the tags the visitor is currently filtering by.
    std::string RenderTagLink(std::string_view selectedTag, const std::vector<std::string>& filter) {
        std::vector<std::string_view> selected{ std::from_range, filter };

        const bool isActive{ std::erase(selected, selectedTag) > 0 };
        if (!isActive)
            selected.emplace_back(selectedTag);

        constexpr auto toTagParam{ [](const std::string_view tag) {
            return "tag=" + Url::Encode(tag);
        } };

        std::string href{ "/?" };
        href.append_range(selected | vs::transform(toTagParam) | vs::join_with('&'));

        return kTagLink.Render(isActive ? " active" : "", href, selectedTag);
    }

    //! @brief One reply. `viewer` decides whether the [delete] button shows up.
    std::string RenderReply(const Reply& reply, const std::optional<Account>& viewer) {
        const std::string actions{ viewer && viewer->Name() == reply.author
            ? kReplyActions.Render(reply.id)
            : std::string{} };

        return kReply.Render(
            reply.id,             // {0}: the anchor, the meta line and the [reply] link reuse it by position
            reply.author,         // {1}
            reply.createdAt,      // {2}
            Html::Raw{ actions }, // {3}
            reply.target,         // {4}: the quote link href and text reuse it by position
            reply.content         // {5}
        );
    }

    //! @brief Header, tag chips, text and its replies. `viewer` decides which buttons show  up, `filter` is the current
    //! tag filter (the tag chips of the post toggle into it).
    std::string RenderPost(const Post& post, const std::optional<Account>& viewer, const std::vector<std::string>& filter) {
        const std::string tags{ std::from_range, post.tags | Thoth::Utils::FlatMap(H_RenderTagLink(filter)) };

        const std::string repliesItems{ std::from_range, post.replies | Thoth::Utils::FlatMap(H_RenderReply(viewer)) };
        const std::string replies{ repliesItems.empty() ? std::string{} : kReplies.Render(Html::Raw{ repliesItems }) };


        // The buttons carry the current text, the script uses it as the default of the edit prompts.
        const std::string actions{ viewer && viewer->Name() == post.author
            ? kPostActions.Render(post.id, post.title, post.content)
            : std::string{} };

        return kPost.Render(
            post.id,              // {0}: the anchor, the meta line and the [reply] link reuse it by position
            post.author,          // {1}
            post.createdAt,       // {2}
            Html::Raw{ actions }, // {3}
            Html::Raw{ tags },    // {4}
            post.title,           // {5}
            post.content,         // {6}
            Html::Raw{ replies }  // {7}
        );
    }

#pragma endregion

}

int main(int argc, char** argv) {
    const std::filesystem::path publicDir{ argc > 1 ? argv[1] : "public" };
    const std::string_view endpointStr{ argc > 2 ? argv[2] : "127.0.0.1:8080" };

#pragma region Fake Database Setup

    Html::Components::Instance().SetRoot(publicDir);

    (void) Accounts::Instance().LogIn("demo", "demo");
    const auto welcome{ Board::Instance().AddPost(
        "demo",
        "Welcome to POST /mortem",
        { "meta", "thoth" },
        "A small board served by Thoth.\n\nEvery route is a typed callback, every page is built from the "
        "components of public/components and nothing is stored on disk. Restart the server and this place "
        "is gone.") };

    (void) Board::Instance().AddReply("demo", welcome, "Click [reply] on any post or reply to answer it.");

#pragma endregion

#pragma region Server Setup

    Hermes::FastIoLoop ioLoop{ std::thread::hardware_concurrency() };
    const auto ip{ Hermes::IpEndpoint::TryParse(endpointStr) };
    if (!ip) {
        std::println("Invalid endpoint: {}", endpointStr);
        return 1;
    }

    Hermes::DefaultSocketData<Hermes::FastIoLoop> listenData{ *ip };
    listenData.executionContext = &ioLoop;

    Server<> server{ std::move(listenData) };

    // The default Host policy only knows "localhost".
    server.hosts.allowed.push_back(std::format("{}", ip->GetIp()));

#pragma endregion

#pragma region Routes


#pragma region Middlewares

    // Creating the middlewares.
    constexpr auto auth{ [](const auto& exchange) -> std::expected<Account, StatusCodeEnum> {
        if (auto user{ Accounts::Instance().WhoAmI(exchange.head) })
            return std::move(*user);

        return std::unexpected{ StatusCodeEnum::Unauthorized };
    } };

    struct TargetId { std::uint64_t value{}; };
    constexpr auto targetId{ [](const auto& exchange) -> std::expected<TargetId, StatusCodeEnum> {
        if (const auto query{ exchange.head.url.GetQueryParams() })
            if (const auto id{ query->GetFrontNonEmpty("id").and_then(ParseId) })
                return TargetId{ *id };


        return std::unexpected{ StatusCodeEnum::BadRequest };
    } };

#pragma endregion


#pragma region Site endpoints (GET only)

    // GET / -> the forum home page. Always a 200. A `?reply=` that points nowhere isn't an error response, it is
    // a notice that the page shows in its <dialog>. The session is optional here, visitors get the login form.
    server->Route("/", Use(auth), [](GetEmptyRequest& request, std::optional<Account> user) -> GetResponseEntry {
        std::optional<std::uint64_t> replyTo{};
        std::vector<std::string> filter{};
        std::string notice{};

        if (const auto query{ request.url.GetQueryParams() }) {
            filter = query->GetCopyNonEmpty("tag").value_or({});

            if (const auto raw{ query->GetFrontNonEmpty("reply") }) {
                replyTo = ParseId(*raw);

                if (!replyTo || !Board::Instance().Exists(*replyTo)) {
                    replyTo.reset();
                    notice = "That post or reply doesn't exist (anymore).";
                }
            }
        }

        const std::string feedItems{ std::from_range,
            Board::Instance().Posts(filter) | Thoth::Utils::FlatMap(H_RenderPost(user, filter)) };

        const std::string feed{
            !feedItems.empty() ? feedItems : filter.empty()
                ? kEmptyFeed.Render("Nothing here yet. Be the first to post.")
                : kEmptyFeed.Render("No post carries every selected tag.")
        };

        const std::string tagLinks{ std::from_range,
            Board::Instance().Tags() | Thoth::Utils::FlatMap(H_RenderTagLink(filter)) };


        // Visitors get the login form and nothing to write with. Logged users get who they are and the compose box,
        // which is a new post form or, after a click on [reply], a form answering that post/reply.
        std::string account{};
        std::string compose{};
        if (user) {
            account = kAccount.Render(user->Name());

            compose = replyTo
                    ? kComposeReply.Render(*replyTo)
                    : kComposePost.Render(Html::Raw{ std::string{ std::from_range,
                        Board::Instance().Tags() | Thoth::Utils::FlatMap([&](auto tag){ return kTagOption.Render(tag); }) } });

        } else {
            account = kLogin.Render();
        }

        return Page(NHeaders::MimeTypes::textHtml, kPage.Render(
            Html::Raw{ feed },                // {0} the feed
            Html::Raw{ account },             // {1} login or account box
            Html::Raw{ compose },             // {2} compose box, empty for visitors
            filter.empty() ? " active" : "",  // {3} highlights the "all" chip
            Html::Raw{ tagLinks },            // {4} the tag chips
            notice                            // {5} text of the <dialog>, it only opens when there is one
        ));
    });


    // static files.
    server->Route("/app.js", [](GetEmptyRequest&) -> GetFileResponse { // direct routing from file
        return Page(NHeaders::MimeTypes::textJavascript, FileBody{ Html::Components::Instance().Root() / "app.js" });
    });

    server->Route("/style.css", [](GetEmptyRequest&) -> GetFileResponse {
        return Page(NHeaders::MimeTypes::textCss, FileBody{ Html::Components::Instance().Root() / "style.css" });
    });

#pragma endregion


#pragma region API endpoints (JSON)

    // POST /api/login -> unknown usernames create the account (simplified auth, see Accounts.hpp). The handler returns
    // a status code (the lib turns it into a response) or a response of its own, both convert to PostResponseEntry.
    server->Route("/api/login", [](const PostRequest& request) -> PostResponseEntry {
        constexpr auto tryLogin{ [](const Json& json) {
            // Missing fields go in as empty and Account::SignUp answers 422.
            const auto username{ json.Get("username").and_then(&Json::EnsureCpy<String>).value_or(String{}) };
            const auto password{ json.Get("password").and_then(&Json::EnsureCpy<String>).value_or(String{}) };

            return Accounts::Instance().LogIn(username.AsRef(), password.AsRef());
        } };

        constexpr auto successfulLoginResponse{ [](const PostMortem::Session& session) {
            PostResponse response{{ .status = StatusCodeEnum::NoContent }};
            response.headers.SetCookie().Add({
                .name     = std::string{ Accounts::kSessionCookie },
                .value    = session.Token(),
                .path     = "/",
                .sameSite = NHeaders::SameSiteEnum::Lax,
                .httpOnly = true,
            });
            return response;
        } };

        return request.AsJson()
                .and_then(tryLogin)
                .transform(successfulLoginResponse);
    });
    // POST /api/logout -> forgets the session and tells the browser to drop the cookie.
    server->Route("/api/logout", [](const PostRequest& request) -> PostResponse {
        for (const auto& [name, value] : request.headers.GetCookiesView())
            if (name == Accounts::kSessionCookie)
                Accounts::Instance().LogOut(value);

        PostResponse response{{ .status = StatusCodeEnum::NoContent }};
        response.headers.SetCookie().Add({
            .name   = std::string{ Accounts::kSessionCookie },
            .maxAge = 0,
            .path   = "/",
        });
        return response;
    });


    // One path accepting POST, PUT and DELETE, any other method is answered 405 by the router and the request body
    // is only read as the declared type. POST asks only for the Account, the ones that point at something also
    // ask for the TargetId (the middlewares are the same, the callback picks the ones it needs, in order).
    server->Match("/api/posts", Use(auth, targetId),

        // POST: a JSON document {"title", "tags": [...], "content"} -> 201 and where the new post lives.
        [](const PostRequest& request, const Account& user) -> PostResponseEntry {
            constexpr auto parseRequest{ [](const Json& json) {
                return Thoth::Utils::TupleOrFirstFail(
                    json.GetOrError("title"  ).and_then(RequiredText<kMaxTitleBytes>),
                    json.GetOrError("tags"   ).and_then(ParseTags),
                    json.GetOrError("content").and_then(RequiredText<kMaxContentBytes>)
                );
            } };
            const auto build{ [&user](auto parsed) {
                auto [title, tags, content]{ std::move(parsed) };
                const std::uint64_t id{ Board::Instance().AddPost(
                user.Name(), std::move(title), std::move(tags), std::move(content)) };

                return std::pair{ StatusCodeEnum::Created, Json{{
                    { "id",   { id } },
                    { "href", { std::format("/#post-{:0>4}", id) } },
                }} };
            } };

            return request.AsJson().and_then(parseRequest).transform(build);
        },

        // PUT: a JSON document {"title", "content"} -> the updated post as JSON.
        [](const Request<PutMethod>& request, Account user, TargetId id) -> ResponseEntry<PutMethod> {
            constexpr auto parseRequest{ [](const Json& json) {
                return Thoth::Utils::TupleOrFirstFail(
                    json.GetOrError("title"  ).and_then(RequiredText<kMaxTitleBytes>),
                    json.GetOrError("content").and_then(RequiredText<kMaxContentBytes>)
                );
            } };
            const auto editPost{ [&user, id](auto parsed) {
                auto [title, content]{ std::move(parsed) };
                return Board::Instance().EditPost(id.value, user.Name(), std::move(title), std::move(content));
            } };

            constexpr auto buildResponse{ [](const Post& post) {
                return Json{{
                    { "id",      Json{ post.id } },
                    { "title",   Json{ post.title } },
                    { "content", Json{ post.content } },
                }};
            } };

            return request.AsJson()
                    .and_then(parseRequest)
                    .and_then(editPost)
                    .transform(buildResponse);
        },

        // DELETE: no body at all (EmptyBody). std::monostate becomes a 204 No Content.
        [](Request<DeleteMethod, EmptyBody>&, const Account& user, const TargetId id) -> ResponseEntry<DeleteMethod> {
            return Board::Instance().DeletePost(id.value, user.Name());
        }
    );

    // Replies have no PUT: (they can't be edited).
    server->Match("/api/replies", Use(auth, targetId),

        // POST: a JSON document {"target": 7, "content"} -> 201, replying to something that doesn't exist is a 404
        // (the Board decides).
        [](const PostRequest& request, const Account& user) -> PostResponseEntry {
            constexpr auto parseRequest{ [](const Json& json) {
                return Thoth::Utils::TupleOrFirstFail(
                    json.GetOrError("target").and_then(&Json::EnsureRefOrError<Number>).and_then(&Number::ExpectUI64),
                    json.GetOrError("content").and_then(RequiredText<kMaxContentBytes>)
                );
            } };
            const auto editPost{ [&user](auto parsed) {
                auto [target, content]{ std::move(parsed) };
                return Board::Instance().AddReply(user.Name(), target, std::move(content));
            } };
            constexpr auto buildResponse{ [](const uint64_t id) {
                return std::pair{ StatusCodeEnum::Created, Json{{
                    { "id",   { id } },
                    { "href", { std::format("/#post-{:0>4}", id) } },
                }} };
            }};

            return request.AsJson()
                    .and_then(parseRequest)
                    .and_then(editPost)
                    .transform(buildResponse);
        },

        [](Request<DeleteMethod, EmptyBody>&, const Account& user, const TargetId id) -> ResponseEntry<DeleteMethod> {
            return Board::Instance().DeleteReply(id.value, user.Name());
        }
    );

#pragma endregion


#pragma endregion

#pragma region Shutdown

    std::println("POST /mortem is up on http://{}, press Enter to stop", endpointStr);
    std::println("Type 'reload' + Enter to re-read public/components without stopping.");
    std::println("Log in as demo / demo, or invent a username: unknown ones are signed up on the spot (toy auth!).");

    std::jthread stopper{ [&server] {
        std::string line{};
        while (std::getline(std::cin, line)) {
            if (line.empty()) // Enter = stop
                break;

            if (line == "reload") {
                Html::Components::Instance().Reload();
                std::println("components reloaded");
                continue;
            }

            std::println("Enter = stop, 'reload' = re-read public/components without stopping");
        }
        server.Stop();
    } };

    try {
        // Run() completes through error (the listener died) or stopped (the Enter above), sync_wait wants a
        // value channel so the stop turns into a plain 0.
        stdexec::sync_wait(server.Run() | stdexec::upon_stopped([] { return 0; }));
        std::println("server stopped");
    } catch (const Thoth::ThothError& error) {
        std::println("server died, {}", error);
        std::println("press Enter to leave");
        stopper.join();
        return 1;
    }

    stopper.join();

#pragma endregion

    return 0;
}
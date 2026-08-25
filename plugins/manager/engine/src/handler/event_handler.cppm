module;
#ifdef CONGELADO_TEST
#include <rfl/Generic.hpp>
#include <rfl/json.hpp>
#endif

export module engine:event_handler;

import std;
import interfaces;
import model;
import shared;
import serde;
import core_logger;
import core_events;
import :context;
#ifdef CONGELADO_TEST
import io_layer_http2;
import boost.ut;
#endif

export namespace engine {

// Routes:
//   GET    /api/v1/event_handlers            → list_handlers
//   POST   /api/v1/event_handlers            → create_handler
//   GET    /api/v1/event_handlers/:name      → get_handler
//   PUT    /api/v1/event_handlers/:name      → update_handler
//   DELETE /api/v1/event_handlers/:name      → remove_handler
// Underscore, not hyphen — the OpenAPI client-SDK generator turns a route's path segments
// straight into C++ namespace identifiers, and a hyphen isn't valid there.
//
// No query-string filtering (e.g. "list by event source") — IRequest has no query-param
// parsing anywhere in this codebase to build on; list_handlers() dumps everything, same
// no-filter convention MetadataHandler's list_task_definitions()/list_workflow_definitions()
// already use, and a caller filters client-side same as the SQL query viewer does.
class EventHandlerHandler {
  public:
    /**
     * @brief Builds a handler bound to the shared EngineContext.
     * @warning Same deferred-callback risk as every other handler in this plugin (see
     * TaskHandler's constructor docs for the full rundown) — captures by reference into
     * Connector callbacks that only run synchronously in local (no-db) mode.
     * @param ctx the engine context to bind; caller keeps it alive for this handler's whole
     * lifetime.
     */
    explicit EventHandlerHandler(EngineContext &ctx) noexcept : m_ctx{ctx} {}

    /**
     * @brief Handles `GET /api/v1/event-handlers` — dumps every stored EventHandler.
     * @param req the inbound request; only its Accept header gets read here.
     * @param res the response this writes the serialized list into.
     */
    void list_handlers(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                       std::function<void()> send) noexcept {
        auto accept = req.find_header("accept");
        m_ctx.get().get_connector().find_all<model::EventHandler>(
            [&res, accept,
             send = std::move(send)](const std::vector<model::EventHandler> &handlers) {
                reply(res, serde::Ser::serialize(accept, handlers));
                send();
            });
    }

    /**
     * @brief Handles `GET /api/v1/event-handlers/:name`.
     * @param req the inbound request; path supplies the name.
     * @param res the response — 200 with the handler, or 404 if nothing matched.
     */
    void get_handler(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                     std::function<void()> send) noexcept {
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto name = std::string{target.substr(target.rfind('/') + 1)};
        m_ctx.get().get_connector().find<model::EventHandler>(
            name, [&res, accept,
                   send = std::move(send)](std::optional<model::EventHandler> result) {
                if (!result) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    send();
                    return;
                }
                reply(res, serde::Ser::serialize(accept, *result));
                send();
            });
    }

    /**
     * @brief Handles `POST /api/v1/event-handlers` — parses, validates, then upserts (so
     * re-registering the same name is idempotent, same reasoning as TaskHandler::
     * create_definition()'s own upsert).
     * @param req the inbound request; body is the handler, Content-Type picks the decoder,
     * Accept picks the reply format.
     * @param res the response — 201 with the created handler, 400 on a parse failure, 422 on a
     * validation failure, or 500 if the upsert fails.
     */
    void create_handler(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                        std::function<void()> send) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::EventHandler>(content_type, body);
        if (!parsed) {
            core::logger::warning("engine", "event-handler/create bad request: {}", parsed.error());
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            send();
            return;
        }
        if (auto validate = parsed->validate(); !validate) {
            core::logger::warning("engine", "event-handler/create invalid: {}", validate.error());
            reply(res, serde::Ser::serialize_error(accept, validate.error()),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            send();
            return;
        }

        model::EventHandler handler = *parsed;
        m_ctx.get().get_connector().upsert<model::EventHandler>(
            handler, [&res, accept, handler, send = std::move(send)](bool oke) {
                if (!oke) {
                    core::logger::error("engine", "event-handler/create db upsert failed");
                    reply(res, serde::Ser::serialize_error(accept, "upsert failed"),
                          interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
                    send();
                    return;
                }
                core::logger::info("engine", "event handler created: '{}'", handler.get_name());
                core::events::publish("engine.event_handler.created",
                                      {{"name", handler.get_name()}});
                reply(res, serde::Ser::serialize(accept, handler),
                      interfaces::io::types::Status::CREATED);
                send();
            });
    }

    /**
     * @brief Handles `PUT /api/v1/event-handlers/:name`.
     * @param req the inbound request; body is the replacement handler.
     * @param res the response — 200 with the updated handler, 400/422 on parse/validation
     * failure, or 404 if `update()` can't find that name.
     */
    void update_handler(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                        std::function<void()> send) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::EventHandler>(content_type, body);
        if (!parsed) {
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            send();
            return;
        }
        if (auto validate = parsed->validate(); !validate) {
            reply(res, serde::Ser::serialize_error(accept, validate.error()),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            send();
            return;
        }

        model::EventHandler handler = *parsed;
        m_ctx.get().get_connector().update<model::EventHandler>(
            handler, [&res, accept, handler, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    send();
                    return;
                }
                reply(res, serde::Ser::serialize(accept, handler));
                send();
            });
    }

    /**
     * @brief Handles `DELETE /api/v1/event-handlers/:name`.
     * @param req the inbound request; path supplies the name.
     * @param res the response — 204 on success, 404 if that name wasn't found.
     */
    void remove_handler(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                        std::function<void()> send) noexcept {
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto name = std::string{target.substr(target.rfind('/') + 1)};
        m_ctx.get().get_connector().remove<model::EventHandler>(
            name, [&res, accept, name, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    send();
                    return;
                }
                core::logger::info("engine", "event handler deleted: '{}'", name);
                core::events::publish("engine.event_handler.deleted", {{"name", name}});
                res.set_status(interfaces::io::types::Status::NO_CONTENT);
                send();
            });
    }

  private:
    std::reference_wrapper<EngineContext> m_ctx;

    static void
    reply(interfaces::io::IResponse &res, std::vector<std::byte> bytes,
          interfaces::io::types::Status status = interfaces::io::types::Status::OK) noexcept {
        res.set_body(std::move(bytes));
        res.set_status(status);
    }

    static std::string flatten_body(interfaces::io::IRequest &req) noexcept {
        std::string out;
        auto &view = req.get_body();
        out.reserve(view.size());
        for (std::byte byte : view) {
            out += static_cast<char>(byte);
        }
        return out;
    }
};

} // namespace engine

#ifdef CONGELADO_TEST
namespace engine::event_handler_tests {
using namespace boost::ut;

// Trivial synchronous in-memory ICache — Connector's find()/write_through()/remove() paths
// abort() via active_cache() if no cache is wired in, so every route below that reaches the
// connector (all but list_handlers/create_handler's-and-update_handler's parse-failure paths)
// needs one of these before it can run at all.
class FakeCache final : public interfaces::ICache {
  public:
    [[nodiscard]] std::string_view backend_name() const noexcept override { return "fake_cache"; }
    void get(std::string_view key, shared::QueryReadFn &&result) noexcept override {
        auto found = m_store.find(std::string{key});
        result(found != m_store.end() ? std::string_view{found->second} : std::string_view{});
    }
    void set(std::string_view key, std::string_view value,
             shared::QueryReadFn &&result) noexcept override {
        m_store[std::string{key}] = std::string{value};
        result("ok");
    }
    void remove(std::string_view key, shared::QueryReadFn &&result) noexcept override {
        m_store.erase(std::string{key});
        result("ok");
    }

  private:
    std::unordered_map<std::string, std::string> m_store;
};

// Minimal real ISerdeFormat, JSON via rfl::json directly (same recipe as json_plugin.cc's own
// JsonPlugin) — needed so list_handlers() below can actually go through serde::Ser::serialize()
// and produce a real, countable JSON array instead of the "no format plugin loaded" error
// payload serialize() falls back to with nothing registered.
class MockJsonFormat final : public interfaces::ISerdeFormat {
  public:
    [[nodiscard]] std::string_view content_type() const noexcept override {
        return "application/json";
    }
    [[nodiscard]] std::string_view format_name() const noexcept override { return "mock-json"; }
    [[nodiscard]] std::expected<std::string, std::string>
    encode(const rfl::Generic &value) const override {
        return rfl::json::write(value);
    }
    [[nodiscard]] std::expected<rfl::Generic, std::string>
    decode(std::string_view data) const override {
        auto result = rfl::json::read<rfl::Generic>(data);
        if (!result) {
            return std::unexpected{result.error().what()};
        }
        return *result;
    }
};

/// @brief Flattens a response's body bytes back into a plain string for content assertions.
[[nodiscard]] std::string body_to_string(interfaces::io::IResponse &res) {
    std::string out;
    auto &view = res.get_body();
    out.reserve(view.size());
    for (std::byte byte : view) {
        out += static_cast<char>(byte);
    }
    return out;
}

suite<"EventHandlerHandler"> event_handler_handler_suite = [] {
    "list_handlers replies 200 with an empty list on a freshly-constructed context"_test = [] {
        engine::EngineContext ctx;
        engine::EventHandlerHandler handler{ctx};
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        bool sent = false;

        handler.list_handlers(req, res, [&sent] { sent = true; });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::OK);
    };

    "list_handlers replies 200 after a handler was upserted directly through the connector"_test =
        [] {
            engine::EngineContext ctx;
            FakeCache cache;
            ctx.set_cache(&cache);
            model::EventHandler seeded;
            seeded.set_name("on_order_shipped");
            seeded.set_event("order_shipped");
            bool upserted = false;
            ctx.get_connector().upsert<model::EventHandler>(
                seeded, [&upserted](bool oke) { upserted = oke; });
            expect(upserted) << fatal;

            engine::EventHandlerHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            bool sent = false;

            handler.list_handlers(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::OK);
        };

    "list_handlers returns every seeded record with no pagination limit applied"_test = [] {
        engine::EngineContext ctx;
        FakeCache cache;
        ctx.set_cache(&cache);
        serde::SerdeFormatRegistry registry;
        registry.add_format(std::make_shared<MockJsonFormat>());
        serde::SerdeFormatRegistry::set_active(&registry);

        constexpr int seeded_count = 50;
        for (int i = 0; i < seeded_count; ++i) {
            model::EventHandler seeded;
            seeded.set_name(std::format("handler_{}", i));
            seeded.set_event("order_shipped");
            bool upserted = false;
            ctx.get_connector().upsert<model::EventHandler>(
                seeded, [&upserted](bool oke) { upserted = oke; });
            expect(upserted) << fatal;
        }

        engine::EventHandlerHandler handler{ctx};
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        // list_handlers() never reads req.get_path() at all (only the Accept header) — a
        // page/limit/offset/event query param here would be silently inert even if IRequest
        // parsed query strings, which it doesn't anywhere in this codebase. Set anyway to
        // document that intent explicitly.
        req.set_header(interfaces::io::types::Token::PATH,
                       "/api/v1/event_handlers?page=1&limit=10&event=order_shipped");
        req.set_header("accept", "application/json");
        bool sent = false;

        handler.list_handlers(req, res, [&sent] { sent = true; });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::OK);
        auto parsed = rfl::json::read<rfl::Generic>(body_to_string(res));
        expect(parsed.has_value()) << fatal;
        auto array = parsed->to_array();
        expect(array.has_value()) << fatal;
        // Every one of the 50 seeded rows comes back, unbounded — pins the "no
        // pagination/limit/event-filter exists" gap for GET /api/v1/event_handlers.
        expect(array->size() == seeded_count);

        serde::SerdeFormatRegistry::set_active(nullptr);
    };

    "get_handler replies 404 for a name that was never stored"_test = [] {
        engine::EngineContext ctx;
        FakeCache cache;
        ctx.set_cache(&cache);
        engine::EventHandlerHandler handler{ctx};
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_header(interfaces::io::types::Token::PATH, "/api/v1/event_handlers/missing");
        bool sent = false;

        handler.get_handler(req, res, [&sent] { sent = true; });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::NOT_FOUND);
    };

    "get_handler replies 200 for a name that was upserted directly through the connector"_test =
        [] {
            engine::EngineContext ctx;
            FakeCache cache;
            ctx.set_cache(&cache);
            model::EventHandler seeded;
            seeded.set_name("on_order_shipped");
            seeded.set_event("order_shipped");
            bool upserted = false;
            ctx.get_connector().upsert<model::EventHandler>(
                seeded, [&upserted](bool oke) { upserted = oke; });
            expect(upserted) << fatal;

            engine::EventHandlerHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            req.set_header(interfaces::io::types::Token::PATH,
                           "/api/v1/event_handlers/on_order_shipped");
            bool sent = false;

            handler.get_handler(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::OK);
        };

    "create_handler replies 400 when the body doesn't parse (no serde format registered)"_test =
        [] {
            serde::SerdeFormatRegistry::set_active(nullptr);
            engine::EngineContext ctx;
            engine::EventHandlerHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            std::vector<std::byte> body{std::byte{'{'}, std::byte{'}'}};
            req.set_body(std::move(body));
            bool sent = false;

            handler.create_handler(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::BAD_REQUEST);
        };

    "update_handler replies 400 when the body doesn't parse (no serde format registered)"_test =
        [] {
            serde::SerdeFormatRegistry::set_active(nullptr);
            engine::EngineContext ctx;
            engine::EventHandlerHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            std::vector<std::byte> body{std::byte{'{'}, std::byte{'}'}};
            req.set_body(std::move(body));
            bool sent = false;

            handler.update_handler(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::BAD_REQUEST);
        };

    "remove_handler replies 404 for a name that was never stored"_test = [] {
        engine::EngineContext ctx;
        FakeCache cache;
        ctx.set_cache(&cache);
        engine::EventHandlerHandler handler{ctx};
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_header(interfaces::io::types::Token::PATH, "/api/v1/event_handlers/missing");
        bool sent = false;

        handler.remove_handler(req, res, [&sent] { sent = true; });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::NOT_FOUND);
    };

    // Item 6 pin: `name` is pulled with `target.substr(target.rfind('/') + 1)` — hand-rolled,
    // zero charset/length validation before it ever reaches the connector. A param with an
    // embedded control character sails through untouched and 404s normally, same as any other
    // never-stored name — nothing rejects it earlier as malformed input.
    "get_handler applies no charset validation — a name with an embedded control character reaches the connector lookup and 404s normally"_test =
        [] {
            engine::EngineContext ctx;
            FakeCache cache;
            ctx.set_cache(&cache);
            engine::EventHandlerHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            req.set_header(interfaces::io::types::Token::PATH,
                           std::string{"/api/v1/event_handlers/weird\x01name"});
            bool sent = false;

            handler.get_handler(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::NOT_FOUND);
        };

    // Same gap, length axis this time — a 4096-character name is just as unvalidated as a
    // normal one, reaching the connector lookup unfiltered.
    "get_handler applies no length validation — a 4096-character name reaches the connector lookup and 404s normally"_test =
        [] {
            engine::EngineContext ctx;
            FakeCache cache;
            ctx.set_cache(&cache);
            engine::EventHandlerHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            std::string long_name(4096, 'a');
            req.set_header(interfaces::io::types::Token::PATH,
                           "/api/v1/event_handlers/" + long_name);
            bool sent = false;

            handler.get_handler(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::NOT_FOUND);
        };

    "remove_handler replies 204 after removing a handler upserted directly through the connector"_test =
        [] {
            engine::EngineContext ctx;
            FakeCache cache;
            ctx.set_cache(&cache);
            model::EventHandler seeded;
            seeded.set_name("on_order_cancelled");
            seeded.set_event("order_cancelled");
            bool upserted = false;
            ctx.get_connector().upsert<model::EventHandler>(
                seeded, [&upserted](bool oke) { upserted = oke; });
            expect(upserted) << fatal;

            engine::EventHandlerHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            req.set_header(interfaces::io::types::Token::PATH,
                           "/api/v1/event_handlers/on_order_cancelled");
            bool sent = false;

            handler.remove_handler(req, res, [&sent] { sent = true; });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::NO_CONTENT);
        };
};

} // namespace engine::event_handler_tests
#endif

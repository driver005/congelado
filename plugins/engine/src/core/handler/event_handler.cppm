export module engine:event_handler;

import std;
import interfaces;
import model;
import shared;
import serde;
import core_logger;
import core_events;
import :context;

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
    void list_handlers(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");
        m_ctx.get().get_connector().find_all<model::EventHandler>(
            [&](const std::vector<model::EventHandler> &handlers) {
                reply(res, serde::Ser::serialize(accept, handlers));
            });
    }

    /**
     * @brief Handles `GET /api/v1/event-handlers/:name`.
     * @param req the inbound request; path supplies the name.
     * @param res the response — 200 with the handler, or 404 if nothing matched.
     */
    void get_handler(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto name = std::string{target.substr(target.rfind('/') + 1)};
        m_ctx.get().get_connector().find<model::EventHandler>(
            name, [&](std::optional<model::EventHandler> result) {
                if (!result) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    return;
                }
                reply(res, serde::Ser::serialize(accept, *result));
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
    void create_handler(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::EventHandler>(content_type, body);
        if (!parsed) {
            core::logger::warning("engine", "event-handler/create bad request: {}", parsed.error());
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            return;
        }
        if (auto validate = parsed->validate(); !validate) {
            core::logger::warning("engine", "event-handler/create invalid: {}", validate.error());
            reply(res, serde::Ser::serialize_error(accept, validate.error()),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            return;
        }

        m_ctx.get().get_connector().upsert<model::EventHandler>(*parsed, [&](bool oke) {
            if (!oke) {
                core::logger::error("engine", "event-handler/create db upsert failed");
                reply(res, serde::Ser::serialize_error(accept, "upsert failed"),
                      interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
                return;
            }
            core::logger::info("engine", "event handler created: '{}'", parsed->get_name());
            core::events::publish("engine.event_handler.created", {{"name", parsed->get_name()}});
            reply(res, serde::Ser::serialize(accept, *parsed),
                  interfaces::io::types::Status::CREATED);
        });
    }

    /**
     * @brief Handles `PUT /api/v1/event-handlers/:name`.
     * @param req the inbound request; body is the replacement handler.
     * @param res the response — 200 with the updated handler, 400/422 on parse/validation
     * failure, or 404 if `update()` can't find that name.
     */
    void update_handler(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::EventHandler>(content_type, body);
        if (!parsed) {
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            return;
        }
        if (auto validate = parsed->validate(); !validate) {
            reply(res, serde::Ser::serialize_error(accept, validate.error()),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            return;
        }

        m_ctx.get().get_connector().update<model::EventHandler>(*parsed, [&](bool oke) {
            if (!oke) {
                reply(res, serde::Ser::serialize_error(accept, "not found"),
                      interfaces::io::types::Status::NOT_FOUND);
                return;
            }
            reply(res, serde::Ser::serialize(accept, *parsed));
        });
    }

    /**
     * @brief Handles `DELETE /api/v1/event-handlers/:name`.
     * @param req the inbound request; path supplies the name.
     * @param res the response — 204 on success, 404 if that name wasn't found.
     */
    void remove_handler(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto name = std::string{target.substr(target.rfind('/') + 1)};
        m_ctx.get().get_connector().remove<model::EventHandler>(name, [&](bool oke) {
            if (!oke) {
                reply(res, serde::Ser::serialize_error(accept, "not found"),
                      interfaces::io::types::Status::NOT_FOUND);
                return;
            }
            core::logger::info("engine", "event handler deleted: '{}'", name);
            core::events::publish("engine.event_handler.deleted", {{"name", name}});
            res.set_status(interfaces::io::types::Status::NO_CONTENT);
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

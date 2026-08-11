export module engine:workflow;

import std;
import interfaces;
import model;
import shared;
import serde;
import core_logger;
import core_events;
import :context;
import :orchestrator;

// ─── WorkflowStartBody ────────────────────────────────────────────────────────

namespace engine {

class WorkflowStartBody {
  public:
    /**
     * @brief Sets the variable bindings to seed a new WorkflowExecution with.
     * @param value the variable key/value pairs to store, moved in.
     */
    void set_variables(std::unordered_map<std::string, std::string> value) noexcept {
        m_variables = std::move(value);
    }
    /**
     * @brief Gets the recorded variable bindings.
     * @return the variable key/value pairs.
     */
    [[nodiscard]] const std::unordered_map<std::string, std::string> &
    get_variables() const noexcept {
        return m_variables;
    }

  private:
    std::unordered_map<std::string, std::string> m_variables;
};

/// @brief Body for `POST /api/v1/workflows/exec/:id/rerun` — which node to reset, and its new
/// input.
class RerunBody {
  public:
    void set_node_ref(std::string value) noexcept { m_node_ref = std::move(value); }
    void set_input(std::unordered_map<std::string, std::string> value) noexcept {
        m_input = std::move(value);
    }
    [[nodiscard]] const std::string &get_node_ref() const noexcept { return m_node_ref; }
    [[nodiscard]] const std::unordered_map<std::string, std::string> &get_input() const noexcept {
        return m_input;
    }

  private:
    std::string m_node_ref;
    std::unordered_map<std::string, std::string> m_input;
};

/// @brief Body for `POST /api/v1/workflows/exec/:id/signal` — which node to signal, and an
/// optional payload.
class SignalBody {
  public:
    void set_node_ref(std::string value) noexcept { m_node_ref = std::move(value); }
    void set_payload(std::optional<std::string> value) noexcept { m_payload = std::move(value); }
    [[nodiscard]] const std::string &get_node_ref() const noexcept { return m_node_ref; }
    [[nodiscard]] const std::optional<std::string> &get_payload() const noexcept {
        return m_payload;
    }

  private:
    std::string m_node_ref;
    std::optional<std::string> m_payload;
};

/// @brief Body for every `POST /api/v1/workflows/bulk/*` route — just the list of executions to
/// apply the action to.
class BulkExecIdsBody {
  public:
    void set_exec_ids(std::vector<std::string> value) noexcept { m_exec_ids = std::move(value); }
    [[nodiscard]] const std::vector<std::string> &get_exec_ids() const noexcept {
        return m_exec_ids;
    }

  private:
    std::vector<std::string> m_exec_ids;
};

/// @brief One execution's outcome in a bulk op's response — mirrors Conductor's own
/// `BulkResponse` shape (per-id success/error), just flattened to a list instead of two parallel
/// collections.
class BulkResult {
  public:
    BulkResult() = default;
    BulkResult(std::string exec_id, bool success) noexcept
        : m_exec_id{std::move(exec_id)}, m_success{success} {}
    void set_exec_id(std::string value) noexcept { m_exec_id = std::move(value); }
    void set_success(bool value) noexcept { m_success = value; }
    [[nodiscard]] const std::string &get_exec_id() const noexcept { return m_exec_id; }
    [[nodiscard]] bool get_success() const noexcept { return m_success; }

  private:
    std::string m_exec_id;
    bool m_success{false};
};

} // namespace engine

template <>
struct serde::Serializable<engine::WorkflowStartBody> {
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"variables", &engine::WorkflowStartBody::get_variables,
                             &engine::WorkflowStartBody::set_variables>{},
        };
    }
};

template <>
struct serde::Serializable<engine::RerunBody> {
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"node_ref", &engine::RerunBody::get_node_ref,
                             &engine::RerunBody::set_node_ref>{},
            serde::FieldDesc<"input", &engine::RerunBody::get_input,
                             &engine::RerunBody::set_input>{},
        };
    }
};

template <>
struct serde::Serializable<engine::SignalBody> {
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"node_ref", &engine::SignalBody::get_node_ref,
                             &engine::SignalBody::set_node_ref>{},
            serde::FieldDesc<"payload", &engine::SignalBody::get_payload,
                             &engine::SignalBody::set_payload>{},
        };
    }
};

template <>
struct serde::Serializable<engine::BulkExecIdsBody> {
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"exec_ids", &engine::BulkExecIdsBody::get_exec_ids,
                             &engine::BulkExecIdsBody::set_exec_ids>{},
        };
    }
};

template <>
struct serde::Serializable<engine::BulkResult> {
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"exec_id", &engine::BulkResult::get_exec_id,
                             &engine::BulkResult::set_exec_id>{},
            serde::FieldDesc<"success", &engine::BulkResult::get_success,
                             &engine::BulkResult::set_success>{},
        };
    }
};

// ─── WorkflowHandler ─────────────────────────────────────────────────────────

export namespace engine {

// Routes:
//   GET    /api/v1/workflows/:name          → get_definition
//   POST   /api/v1/workflows                → create_definition
//   PUT    /api/v1/workflows/:name          → update_definition
//   DELETE /api/v1/workflows/:name          → remove_definition
//   POST   /api/v1/workflows/:name/start    → start_execution
//   GET    /api/v1/workflows/exec/:id       → get_execution
//   DELETE /api/v1/workflows/exec/:id       → terminate_execution
class WorkflowHandler {
  public:
    /**
     * @brief Builds a handler bound to the shared EngineContext — no state of its own, every
     * route below leans on `m_ctx` to reach the connector.
     * @warning Same deferred-callback risk as TaskHandler (see its constructor docs for the
     * full rundown): every route method below captures locals like `accept`/`target`/`name`
     * by reference into the callback handed to Connector::find/insert/update(), and that
     * callback only runs synchronously when no database is configured. With a real database
     * wired in it's deferred to a later tick, after the handler's already returned — dangling
     * stack references waiting to happen. terminate_execution() below has it worst: it reads
     * `found`/`handled` right after calling find(), assuming the callback already ran.
     * @param ctx the engine context to bind; caller keeps it alive for this handler's whole
     * lifetime.
     */
    explicit WorkflowHandler(EngineContext &ctx) noexcept : m_ctx(ctx) {}

    /**
     * @brief Handles `GET /api/v1/workflows/:name` — looks up one WorkflowDef by name.
     * @warning `name` is pulled by slicing everything after the last `/` in the path, no real
     * path-param extraction — same low-effort parsing this whole file leans on throughout.
     * @param req the inbound request; path supplies the name, Accept header picks the format.
     * @param res the response — 200 with the definition, or 404 if nothing matched.
     */
    void get_definition(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                        std::function<void()> send) noexcept {
        // slice the name off the tail of the path — no dedicated route-param binding here either
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto name = std::string{target.substr(target.rfind('/') + 1)};

        // look it up and let the callback decide 404 vs a normal 200 reply. Not noexcept: it
        // calls reply()/serde::Ser::serialize(), which allocate and can throw. Connector::find()'s
        // callback parameter (std::move_only_function<void(...)>) doesn't require noexcept, nor
        // does interfaces::HandlerFn (std::function), so dropping it here is safe.
        m_ctx.get().get_connector().find<model::WorkflowDef>(
            name, [&res, accept, send = std::move(send)](std::optional<model::WorkflowDef> result) {
                if (!result) {
                    // nothing under that name — bounce a 404
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
     * @brief Handles `POST /api/v1/workflows` — parses, validates, then inserts a new
     * WorkflowDef.
     * @note Bad-request and validation-failure paths log a warning before replying — every
     * rejected create leaves a trace, W for debuggability.
     * @param req the inbound request; body is the definition, Content-Type picks the decoder,
     * Accept picks the reply format.
     * @param res the response — 201 with the created definition, 400 on a parse failure, 422 on
     * a validation failure, or 500 if the insert itself fails.
     */
    // Not noexcept: string/serde operations below (deserialize, validate, serialize_error) can
    // throw. interfaces::HandlerFn is a std::function, which doesn't require a noexcept target,
    // and every route lambda in routes.cppm that calls this isn't noexcept either.
    void create_definition(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                           std::function<void()> send) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        // decode the body into a WorkflowDef — bail with a 400 if it doesn't even parse
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::WorkflowDef>(content_type, body);
        if (!parsed) {
            core::logger::warning("engine", "wf/create bad request: {}", parsed.error());
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            send();
            return;
        }

        // domain validation before anything touches the store — 422 if it's not clean
        if (auto value = parsed->validate(); !value) {
            core::logger::warning("engine", "wf/create invalid: {}", value.error());
            reply(res, serde::Ser::serialize_error(accept, value.error()),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            send();
            return;
        }

        // parsed and validated — insert it and reply with what got created. Callback calls
        // logger::error/info and serde::Ser::serialize(), both of which can throw; not
        // noexcept, same reasoning as create_definition() itself above.
        model::WorkflowDef definition = *parsed;
        m_ctx.get().get_connector().insert<model::WorkflowDef>(
            definition, [&res, accept, definition, send = std::move(send)](bool oke) {
                if (!oke) {
                    core::logger::error("engine", "wf/create db insert failed");
                    reply(res, serde::Ser::serialize_error(accept, "insert failed"),
                          interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
                    send();
                    return;
                }
                core::logger::info("engine", "workflow created: '{}'", definition.get_name());
                core::events::publish("engine.workflow_def.created",
                                      {{"name", definition.get_name()}});
                reply(res, serde::Ser::serialize(accept, definition),
                      interfaces::io::types::Status::CREATED);
                send();
            });
    }

    /**
     * @brief Handles `PUT /api/v1/workflows/:name` — parses, validates, then updates an
     * existing WorkflowDef.
     * @note `parsed->get_name()` from the decoded body is what's actually used as the update
     * key, not the `:name` path segment — same quirk as TaskHandler::update_definition(). A
     * body whose name doesn't match the URL just updates (or 404s on) whatever name the body
     * carries.
     * @warning Unlike TaskHandler::create_definition()/update_definition(), a bad-request or
     * validation failure here doesn't log a warning first — silent 400/422, no trace left
     * behind. Inconsistent with the sibling handler, worth knowing if you're chasing a "why
     * didn't this show up in the logs" L.
     * @param req the inbound request; body is the replacement definition, Content-Type picks
     * the decoder, Accept picks the reply format.
     * @param res the response — 200 with the updated definition, 400 on a parse failure, 422 on
     * a validation failure, or 404 if the connector's update() can't find that name.
     */
    // Not noexcept — same reasoning as create_definition() above.
    void update_definition(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                           std::function<void()> send) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        // decode the replacement body — 400 if it's not a valid WorkflowDef (no warning logged
        // here, unlike TaskHandler::update_definition())
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::WorkflowDef>(content_type, body);
        if (!parsed) {
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            send();
            return;
        }

        // same validation pass as create — 422 if it fails
        if (auto value = parsed->validate(); !value) {
            reply(res, serde::Ser::serialize_error(accept, value.error()),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            send();
            return;
        }

        // key off the name in the body, not the URL segment — update() 404s if it's not there.
        // Callback calls serde::Ser::serialize()/serialize_error(), which can throw; not
        // noexcept, same reasoning as the insert() callback above.
        model::WorkflowDef definition = *parsed;
        m_ctx.get().get_connector().update<model::WorkflowDef>(
            definition, [&res, accept, definition, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    send();
                    return;
                }
                reply(res, serde::Ser::serialize(accept, definition));
                send();
            });
    }

    /**
     * @brief Handles `DELETE /api/v1/workflows/:name` — removes a WorkflowDef by name.
     * @warning Same last-segment path slicing as get_definition() — no dedicated param
     * extraction here either.
     * @param req the inbound request; path supplies the name, Accept header picks the format.
     * @param res the response — 204 on success, 404 if that name wasn't found.
     */
    void remove_definition(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                           std::function<void()> send) noexcept {
        // same tail-slicing move as get_definition to pull the name back out
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto name = std::string{target.substr(target.rfind('/') + 1)};

        // delete by name — 404 if there was nothing to delete, else 204. Callback calls
        // serde::Ser::serialize_error(), which can throw; not noexcept, same reasoning as the
        // other callbacks in this class.
        m_ctx.get().get_connector().remove<model::WorkflowDef>(
            name, [&res, accept, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    send();
                    return;
                }
                res.set_status(interfaces::io::types::Status::NO_CONTENT);
                send();
            });
    }

    // Path: /api/v1/workflows/:name/start — def_name is the segment before "start".
    /**
     * @brief Handles `POST /api/v1/workflows/:name/start` — delegates to Orchestrator::start(),
     * which 404s if `:name` doesn't exist, otherwise spins up a new RUNNING WorkflowExecution,
     * immediately spawns its DAG's start nodes, and persists the result.
     * @warning `def_name` is carved out by hand via two `rfind('/')` calls, same brittle
     * pattern as TaskHandler::enqueue_task()/submit_result().
     * @warning A non-empty but malformed body gets swallowed silently — if deserialize() fails,
     * `variables` just stays empty instead of the request getting a 400. Same body-swallowing
     * move as TaskHandler::enqueue_task().
     * @param req the inbound request; path supplies the definition name, an optional JSON body
     * supplies variable bindings, Accept picks the reply format.
     * @param res the response — 202 with the new (already-advanced) execution, or 404 if
     * `:name` doesn't name a real WorkflowDef.
     */
    // Not noexcept — model construction, serde::Ser::deserialize()/serialize(), and
    // std::chrono formatting below can throw. Same reasoning as create_definition() above.
    void start_execution(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                         std::function<void()> send) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        // def_name sits between the last two slashes — hand-rolled, no route-param extraction
        auto target = req.get_path();
        auto last = target.rfind('/');
        auto before = target.rfind('/', last > 0 ? last - 1 : 0);
        auto def_name = std::string{target.substr(before + 1, last - before - 1)};

        // optional JSON body seeds the variable bindings — malformed body just leaves it empty
        std::unordered_map<std::string, std::string> variables;
        auto body = flatten_body(req);
        if (!body.empty()) {
            if (auto parsed = serde::Ser::deserialize<WorkflowStartBody>(content_type, body)) {
                variables = parsed->get_variables();
            }
        }

        Orchestrator{m_ctx.get()}.start(
            def_name, std::move(variables), std::nullopt,
            [&res, accept, def_name,
             send = std::move(send)](std::optional<model::WorkflowExecution> exec) mutable {
                if (!exec) {
                    core::logger::warning("engine", "wf/start not found: '{}'", def_name);
                    reply(res, serde::Ser::serialize_error(accept, "workflow definition not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    send();
                    return;
                }
                core::logger::info("engine", "exec '{}' started for '{}'", exec->get_exec_id(),
                                   def_name);
                core::events::publish("engine.workflow.started",
                                      {{"exec_id", std::format("{}", exec->get_exec_id())},
                                       {"workflow_name", def_name}});
                reply(res, serde::Ser::serialize(accept, *exec),
                      interfaces::io::types::Status::ACCEPTED);
                send();
            });
    }

    // Path: /api/v1/workflows/exec/:id — exec_id is the last segment.
    /**
     * @brief Handles `GET /api/v1/workflows/exec/:id` — looks up one WorkflowExecution by id.
     * @param req the inbound request; path supplies the execution id, Accept header picks the
     * format.
     * @param res the response — 200 with the execution, or 404 if that id wasn't found.
     */
    void get_execution(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                       std::function<void()> send) noexcept {
        // exec_id is the last path segment
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto exec_id_str = std::string{target.substr(target.rfind('/') + 1)};

        // look it up and let the callback decide 404 vs 200. Not noexcept — same reasoning as
        // get_definition()'s find() callback above.
        m_ctx.get().get_connector().find<model::WorkflowExecution>(
            exec_id_str, [this, &res, accept,
                          send = std::move(send)](std::optional<model::WorkflowExecution> result) {
                if (!result) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    send();
                    return;
                }
                mask_and_reply(res, accept, std::move(send), std::move(*result));
            });
    }

    // Path: /api/v1/workflows/exec/:id — exec_id is the last segment.
    /**
     * @brief Handles `DELETE /api/v1/workflows/exec/:id` — terminates a running
     * WorkflowExecution, rejecting the request if the execution is already in a terminal
     * state.
     * @note The whole terminate flow (404 / already-terminal / flip-and-persist) runs inside
     * find()'s callback chain, not synchronously after it — with a real database backend find()
     * defers its callback to a later tick, so reading a result set by that callback synchronously
     * after the find() call (the shape this used to have) saw an empty optional and crashed. Every
     * local the callbacks need is captured by copy/move, never [&]-by-reference to this stack frame.
     * @param req the inbound request; path supplies the execution id, Accept header picks the
     * format.
     * @param res the response — 200 with the terminated execution, 404 if the id wasn't found
     * (on the initial lookup or the follow-up update()), or 409 if it was already terminal.
     */
    void terminate_execution(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                             std::function<void()> send) {
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto exec_id_str = std::string{target.substr(target.rfind('/') + 1)};

        // look the execution up — the callback handles the 404, already-terminal, and
        // flip-and-persist cases inline. Not noexcept: calls logger::warning() and
        // serde::Ser::serialize_error(), both of which can throw.
        m_ctx.get().get_connector().find<model::WorkflowExecution>(
            exec_id_str, [this, &res, exec_id_str, accept, send = std::move(send)](
                             std::optional<model::WorkflowExecution> result) mutable {
                if (!result) {
                    // no such execution — 404
                    core::logger::warning("engine", "wf/terminate not found: '{}'", exec_id_str);
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    send();
                    return;
                }
                if (model::is_terminal(result->get_status())) {
                    // already done/failed/terminated — refuse to terminate it twice, no cap
                    core::logger::warning("engine", "wf/terminate already terminal: '{}'",
                                          exec_id_str);
                    reply(res, serde::Ser::serialize_error(accept, "already in terminal state"),
                          interfaces::io::types::Status::CONFLICT);
                    send();
                    return;
                }

                // still running — flip to TERMINATED and persist it
                auto execution = std::move(*result);
                execution.set_status(model::WorkflowStatus::TERMINATED);
                // Callback calls logger::info() and serde::Ser::serialize(), both of which can
                // throw; not noexcept, same reasoning as the other connector callbacks in this
                // class.
                m_ctx.get().get_connector().update<model::WorkflowExecution>(
                    execution, [this, &res, exec_id_str, accept, execution,
                                send = std::move(send)](bool oke) mutable {
                        if (!oke) {
                            reply(res, serde::Ser::serialize_error(accept, "not found"),
                                  interfaces::io::types::Status::NOT_FOUND);
                            send();
                            return;
                        }
                        core::logger::info("engine", "exec terminated: '{}'", exec_id_str);
                        core::events::publish("engine.workflow.terminated",
                                              {{"exec_id", exec_id_str}});
                        Orchestrator{m_ctx.get()}.on_execution_terminal(execution);
                        reply(res, serde::Ser::serialize(accept, execution));
                        send();
                    });
            });
    }

    /// @brief Path is `/api/v1/workflows/exec/:id/{pause,resume,retry,restart,rerun,signal}` —
    /// `:id` sits between the last two slashes, same brittle hand-rolled slicing as everywhere
    /// else in this file.
    static std::string exec_id_from_action_path(interfaces::io::IRequest &req) {
        auto target = req.get_path();
        auto last = target.rfind('/');
        auto before = target.rfind('/', last > 0 ? last - 1 : 0);
        return std::string{target.substr(before + 1, last - before - 1)};
    }

    /**
     * @brief Handles `POST /api/v1/workflows/exec/:id/pause`.
     * @param req the inbound request; path supplies the execution id.
     * @param res the response — 200 on success, 409 if the execution wasn't found or wasn't
     * RUNNING.
     */
    void pause_execution(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                         std::function<void()> send) {
        auto accept = req.find_header("accept");
        Orchestrator{m_ctx.get()}.pause(
            exec_id_from_action_path(req), [&res, accept, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(res, serde::Ser::serialize_error(accept, "not found or not running"),
                          interfaces::io::types::Status::CONFLICT);
                    send();
                    return;
                }
                res.set_status(interfaces::io::types::Status::OK);
                send();
            });
    }

    /**
     * @brief Handles `POST /api/v1/workflows/exec/:id/resume`.
     * @param req the inbound request; path supplies the execution id.
     * @param res the response — 200 on success, 409 if the execution wasn't found or wasn't
     * PAUSED.
     */
    void resume_execution(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                          std::function<void()> send) {
        auto accept = req.find_header("accept");
        Orchestrator{m_ctx.get()}.resume(
            exec_id_from_action_path(req), [&res, accept, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(res, serde::Ser::serialize_error(accept, "not found or not paused"),
                          interfaces::io::types::Status::CONFLICT);
                    send();
                    return;
                }
                res.set_status(interfaces::io::types::Status::OK);
                send();
            });
    }

    /**
     * @brief Handles `POST /api/v1/workflows/exec/:id/retry`.
     * @param req the inbound request; path supplies the execution id.
     * @param res the response — 200 on success, 409 if the execution/def wasn't found, wasn't
     * FAILED, or the def forbids retry (restartable == false).
     */
    void retry_execution(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                         std::function<void()> send) {
        auto accept = req.find_header("accept");
        Orchestrator{m_ctx.get()}.retry(
            exec_id_from_action_path(req), [&res, accept, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(res, serde::Ser::serialize_error(accept, "not retryable"),
                          interfaces::io::types::Status::CONFLICT);
                    send();
                    return;
                }
                res.set_status(interfaces::io::types::Status::OK);
                send();
            });
    }

    /**
     * @brief Handles `POST /api/v1/workflows/exec/:id/restart`.
     * @param req the inbound request; path supplies the execution id.
     * @param res the response — 200 on success, 409 if the execution/def wasn't found, wasn't in
     * a terminal state, or the def forbids restart.
     */
    void restart_execution(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                           std::function<void()> send) {
        auto accept = req.find_header("accept");
        Orchestrator{m_ctx.get()}.restart(
            exec_id_from_action_path(req), [&res, accept, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(res, serde::Ser::serialize_error(accept, "not restartable"),
                          interfaces::io::types::Status::CONFLICT);
                    send();
                    return;
                }
                res.set_status(interfaces::io::types::Status::OK);
                send();
            });
    }

    /**
     * @brief Handles `POST /api/v1/workflows/exec/:id/rerun` — body names which node to reset
     * and its new input. See Orchestrator::rerun()'s own docs for how this differs from
     * Conductor's full clone-and-replay semantics.
     * @param req the inbound request; path supplies the execution id, body supplies node_ref +
     * input.
     * @param res the response — 200 on success, 400 on a parse failure, 409 if the
     * execution/def/node wasn't found or the def forbids it.
     */
    void rerun_execution(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                         std::function<void()> send) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<RerunBody>(content_type, body);
        if (!parsed) {
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            send();
            return;
        }
        Orchestrator{m_ctx.get()}.rerun(
            exec_id_from_action_path(req), parsed->get_node_ref(), parsed->get_input(),
            [&res, accept, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(res, serde::Ser::serialize_error(accept, "not rerunnable"),
                          interfaces::io::types::Status::CONFLICT);
                    send();
                    return;
                }
                res.set_status(interfaces::io::types::Status::OK);
                send();
            });
    }

    /**
     * @brief Handles `POST /api/v1/workflows/exec/:id/signal` — wakes an indefinite WAIT or a
     * HUMAN instance waiting on `node_ref`. The generic external-signal completion path for
     * anything that isn't going through `POST /api/v1/tasks/:id/result` directly.
     * @param req the inbound request; path supplies the execution id, body supplies node_ref +
     * an optional payload.
     * @param res the response — 200 on success, 400 on a parse failure, 409 if no matching
     * IN_PROGRESS instance was found.
     */
    void signal_execution(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                          std::function<void()> send) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<SignalBody>(content_type, body);
        if (!parsed) {
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            send();
            return;
        }
        Orchestrator{m_ctx.get()}.signal(
            exec_id_from_action_path(req), parsed->get_node_ref(), parsed->get_payload(),
            [&res, accept, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(res,
                          serde::Ser::serialize_error(accept, "no matching in-progress instance"),
                          interfaces::io::types::Status::CONFLICT);
                    send();
                    return;
                }
                res.set_status(interfaces::io::types::Status::OK);
                send();
            });
    }

    /// @brief Handles `POST /api/v1/workflows/bulk/pause` — body is a BulkExecIdsBody, response
    /// a `vector<BulkResult>` with one entry per exec_id, applied sequentially (not in
    /// parallel — Connector's own op queue serializes them anyway in db-backed mode, and this
    /// keeps the per-id success bookkeeping trivial).
    void bulk_pause(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                    std::function<void()> send) {
        bulk_dispatch(req, res, std::move(send),
                      [](Orchestrator &orchestrator, std::string exec_id,
                         std::move_only_function<void(bool)> callback) {
                          orchestrator.pause(std::move(exec_id), std::move(callback));
                      });
    }

    /// @brief Handles `POST /api/v1/workflows/bulk/resume`.
    void bulk_resume(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                     std::function<void()> send) {
        bulk_dispatch(req, res, std::move(send),
                      [](Orchestrator &orchestrator, std::string exec_id,
                         std::move_only_function<void(bool)> callback) {
                          orchestrator.resume(std::move(exec_id), std::move(callback));
                      });
    }

    /// @brief Handles `POST /api/v1/workflows/bulk/retry`.
    void bulk_retry(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                    std::function<void()> send) {
        bulk_dispatch(req, res, std::move(send),
                      [](Orchestrator &orchestrator, std::string exec_id,
                         std::move_only_function<void(bool)> callback) {
                          orchestrator.retry(std::move(exec_id), std::move(callback));
                      });
    }

    /// @brief Handles `POST /api/v1/workflows/bulk/restart`.
    void bulk_restart(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                      std::function<void()> send) {
        bulk_dispatch(req, res, std::move(send),
                      [](Orchestrator &orchestrator, std::string exec_id,
                         std::move_only_function<void(bool)> callback) {
                          orchestrator.restart(std::move(exec_id), std::move(callback));
                      });
    }

    /// @brief Handles `POST /api/v1/workflows/bulk/terminate`.
    void bulk_terminate(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                        std::function<void()> send) {
        bulk_dispatch(req, res, std::move(send),
                      [](Orchestrator &orchestrator, std::string exec_id,
                         std::move_only_function<void(bool)> callback) {
                          orchestrator.terminate(std::move(exec_id), std::move(callback));
                      });
    }

    /// @brief Handles `POST /api/v1/workflows/bulk/remove` — the only bulk op that isn't an
    /// Orchestrator method (removal is plain storage cleanup, no cascade/propagation involved),
    /// so this reaches straight into the connector instead.
    void bulk_remove(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                     std::function<void()> send) {
        bulk_dispatch(req, res, std::move(send),
                      [this](Orchestrator & /*orchestrator*/, std::string exec_id,
                             std::move_only_function<void(bool)> callback) {
                          m_ctx.get().get_connector().remove<model::WorkflowExecution>(
                              exec_id, std::move(callback));
                      });
    }

  private:
    std::reference_wrapper<EngineContext> m_ctx;

    /**
     * @brief Redacts every masked field (per each instance's own TaskDef.masked_fields) out of
     * `exec`'s embedded task_instances before replying — response-time-only, never touches what
     * gets persisted. One `find_all<TaskDef>` covers every instance regardless of how many
     * distinct defs it spans, rather than a lookup per instance.
     * @param res the response to reply into.
     * @param accept the Accept header value picking the reply format.
     * @param exec the execution to mask and reply with; consumed by value since its
     * task_instances get mutated in place before serializing.
     */
    void mask_and_reply(interfaces::io::IResponse &res, std::string_view accept,
                        std::function<void()> send, model::WorkflowExecution exec) {
        m_ctx.get().get_connector().find_all<model::TaskDef>(
            [&res, accept, send = std::move(send),
             exec = std::move(exec)](std::vector<model::TaskDef> defs) mutable {
                std::unordered_map<std::string, std::vector<std::string>> masks;
                for (auto const &def : defs) {
                    if (!def.get_masked_fields().empty()) {
                        masks[def.get_name()] = def.get_masked_fields();
                    }
                }
                if (!masks.empty()) {
                    auto instances = exec.get_task_instances();
                    for (auto &instance : instances) {
                        auto mask_it = masks.find(instance.get_def_name());
                        if (mask_it == masks.end()) {
                            continue;
                        }
                        auto input = instance.get_input_data();
                        auto output = instance.get_output_data();
                        for (auto const &field : mask_it->second) {
                            if (input.contains(field)) {
                                input[field] = "*******";
                            }
                            if (output.contains(field)) {
                                output[field] = "*******";
                            }
                        }
                        instance.set_input_data(std::move(input));
                        instance.set_output_data(std::move(output));
                    }
                    exec.set_task_instances(std::move(instances));
                }
                reply(res, serde::Ser::serialize(accept, exec));
                send();
            });
    }

    /**
     * @brief Shared sequential-apply machinery for every `POST /api/v1/workflows/bulk/` route:
     * parses the BulkExecIdsBody, then applies `op` to each exec_id one at a time (not in
     * parallel), collecting a BulkResult per id, and replies with the full list once done.
     * @param req the inbound request; body is a BulkExecIdsBody.
     * @param res the response — 200 with the per-id results, or 400 on a parse failure.
     * @param op the per-execution action; takes a fresh Orchestrator (cheap — just wraps
     * `m_ctx`), the exec_id, and a callback for whether it succeeded.
     */
    void bulk_dispatch(
        interfaces::io::IRequest &req, interfaces::io::IResponse &res, std::function<void()> send,
        std::function<void(Orchestrator &, std::string, std::move_only_function<void(bool)>)> op) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<BulkExecIdsBody>(content_type, body);
        if (!parsed) {
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            send();
            return;
        }
        bulk_apply(parsed->get_exec_ids(), 0, {}, std::move(op), res, accept, std::move(send));
    }

    void bulk_apply(
        std::vector<std::string> exec_ids, std::size_t index, std::vector<BulkResult> results,
        std::function<void(Orchestrator &, std::string, std::move_only_function<void(bool)>)> op,
        interfaces::io::IResponse &res, std::string_view accept, std::function<void()> send) {
        if (index >= exec_ids.size()) {
            reply(res, serde::Ser::serialize(accept, results));
            send();
            return;
        }
        auto exec_id = exec_ids[index];
        Orchestrator orchestrator{m_ctx.get()};
        op(orchestrator, exec_id,
           [this, exec_ids = std::move(exec_ids), index, results = std::move(results),
            op = std::move(op), &res, accept, send = std::move(send)](bool oke) mutable {
               results.emplace_back(exec_ids[index], oke);
               bulk_apply(std::move(exec_ids), index + 1, std::move(results), std::move(op), res,
                          accept, std::move(send));
           });
    }

    /**
     * @brief Shared reply helper — writes `bytes` into the response body and sets the status,
     * defaulting to 200 OK when the caller doesn't hand over anything else.
     * @param res the response to fill in.
     * @param bytes the body bytes to write.
     * @param status the status code to set, defaults to OK.
     */
    static void
    reply(interfaces::io::IResponse &res, std::vector<std::byte> bytes,
          interfaces::io::types::Status status = interfaces::io::types::Status::OK) noexcept {
        res.set_body(std::move(bytes));
        res.set_status(status);
    }

    /**
     * @brief Copies the request body's byte view out into a plain `std::string`, one
     * `static_cast<char>` per byte — same flattening every handler above needs before handing
     * text off to serde::Ser::deserialize().
     * @param req the request whose body gets flattened.
     * @return the body bytes reinterpreted as a string, same length, same order.
     */
    static std::string flatten_body(interfaces::io::IRequest &req) noexcept {
        std::string out;
        auto &view = req.get_body();
        out.reserve(view.size());
        // walk the raw bytes one at a time, reinterpreting each as a char
        for (std::byte byte : view) {
            out += static_cast<char>(byte);
        }
        return out;
    }
};

} // namespace engine

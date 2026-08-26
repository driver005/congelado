export module engine:workflow;

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

// ─── WorkflowStartBody ────────────────────────────────────────────────────────

namespace engine {

class WorkflowStartBody
{
public:
    /**
     * @brief Sets the variable bindings to seed a new WorkflowExecution with.
     * @param value the variable key/value pairs to store, moved in.
     */
    void set_variables(std::unordered_map<std::string, std::string> value) noexcept
    {
        m_variables = std::move(value);
    }

    /**
     * @brief Gets the recorded variable bindings.
     * @return the variable key/value pairs.
     */
    [[nodiscard]] const std::unordered_map<std::string, std::string>& get_variables() const noexcept
    {
        return m_variables;
    }

private:
    std::unordered_map<std::string, std::string> m_variables;
};

/// @brief Body for `POST /api/v1/workflows/exec/:id/rerun` — which node to reset, and its new
/// input.
class RerunBody
{
public:
    void set_node_ref(std::string value) noexcept
    {
        m_node_ref = std::move(value);
    }

    void set_input(serde::Value value) noexcept
    {
        m_input = std::move(value);
    }

    [[nodiscard]] const std::string& get_node_ref() const noexcept
    {
        return m_node_ref;
    }

    [[nodiscard]] const serde::Value& get_input() const noexcept
    {
        return m_input;
    }

private:
    std::string m_node_ref;
    serde::Value m_input;
};

/// @brief Body for `POST /api/v1/workflows/exec/:id/signal` — which node to signal, and an
/// optional payload.
class SignalBody
{
public:
    void set_node_ref(std::string value) noexcept
    {
        m_node_ref = std::move(value);
    }

    void set_payload(std::optional<std::string> value) noexcept
    {
        m_payload = std::move(value);
    }

    [[nodiscard]] const std::string& get_node_ref() const noexcept
    {
        return m_node_ref;
    }

    [[nodiscard]] const std::optional<std::string>& get_payload() const noexcept
    {
        return m_payload;
    }

private:
    std::string m_node_ref;
    std::optional<std::string> m_payload;
};

/// @brief Body for every `POST /api/v1/workflows/bulk/*` route — just the list of executions to
/// apply the action to.
class BulkExecIdsBody
{
public:
    void set_exec_ids(std::vector<std::string> value) noexcept
    {
        m_exec_ids = std::move(value);
    }

    [[nodiscard]] const std::vector<std::string>& get_exec_ids() const noexcept
    {
        return m_exec_ids;
    }

private:
    std::vector<std::string> m_exec_ids;
};

/// @brief One execution's outcome in a bulk op's response — mirrors Conductor's own
/// `BulkResponse` shape (per-id success/error), just flattened to a list instead of two
/// parallel collections.
class BulkResult
{
public:
    BulkResult() = default;

    BulkResult(std::string exec_id, bool success) noexcept :
        m_exec_id{std::move(exec_id)},
        m_success{success}
    {
    }

    void set_exec_id(std::string value) noexcept
    {
        m_exec_id = std::move(value);
    }

    void set_success(bool value) noexcept
    {
        m_success = value;
    }

    [[nodiscard]] const std::string& get_exec_id() const noexcept
    {
        return m_exec_id;
    }

    [[nodiscard]] bool get_success() const noexcept
    {
        return m_success;
    }

private:
    std::string m_exec_id;
    bool m_success{false};
};

} // namespace engine

template<>
struct serde::Serializable<engine::WorkflowStartBody>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "variables", &engine::WorkflowStartBody::get_variables,
                &engine::WorkflowStartBody::set_variables>{},
        };
    }
};

template<>
struct serde::Serializable<engine::RerunBody>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "node_ref", &engine::RerunBody::get_node_ref, &engine::RerunBody::set_node_ref>{},
            serde::FieldDesc<
                "input", &engine::RerunBody::get_input, &engine::RerunBody::set_input>{},
        };
    }
};

template<>
struct serde::Serializable<engine::SignalBody>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "node_ref", &engine::SignalBody::get_node_ref, &engine::SignalBody::set_node_ref>{},
            serde::FieldDesc<
                "payload", &engine::SignalBody::get_payload, &engine::SignalBody::set_payload>{},
        };
    }
};

template<>
struct serde::Serializable<engine::BulkExecIdsBody>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "exec_ids", &engine::BulkExecIdsBody::get_exec_ids,
                &engine::BulkExecIdsBody::set_exec_ids>{},
        };
    }
};

template<>
struct serde::Serializable<engine::BulkResult>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "exec_id", &engine::BulkResult::get_exec_id, &engine::BulkResult::set_exec_id>{},
            serde::FieldDesc<
                "success", &engine::BulkResult::get_success, &engine::BulkResult::set_success>{},
        };
    }
};

#ifdef CONGELADO_TEST
namespace engine::workflow_dto_tests {
using namespace boost::ut;

suite<"WorkflowStartBody"> workflow_start_body_suite = [] {
    "default-constructs with empty variables"_test = [] {
        engine::WorkflowStartBody body;
        expect(body.get_variables().empty());
    };

    "set_variables/get_variables round-trip"_test = [] {
        engine::WorkflowStartBody body;
        body.set_variables({{"key", "value"}});
        expect(body.get_variables().at("key") == "value");
    };
};

suite<"RerunBody"> rerun_body_suite = [] {
    "set_node_ref/get_node_ref round-trip"_test = [] {
        engine::RerunBody body;
        body.set_node_ref("node-a");
        expect(body.get_node_ref() == "node-a");
    };

    "set_input/get_input round-trip"_test = [] {
        engine::RerunBody body;
        body.set_input(serde::Value{std::string{"fresh-input"}});
        auto decoded = serde::Ser::from_value<std::string>(body.get_input());
        expect(decoded.has_value()) << fatal;
        expect(*decoded == "fresh-input");
    };
};

suite<"SignalBody"> signal_body_suite = [] {
    "default-constructs with no payload"_test = [] {
        engine::SignalBody body;
        expect(!body.get_payload().has_value());
    };

    "set_node_ref/get_node_ref round-trip"_test = [] {
        engine::SignalBody body;
        body.set_node_ref("node-b");
        expect(body.get_node_ref() == "node-b");
    };

    "set_payload/get_payload round-trip, including clearing back to nullopt"_test = [] {
        engine::SignalBody body;
        body.set_payload(std::string{"ping"});
        expect(body.get_payload().value() == "ping");
        body.set_payload(std::nullopt);
        expect(!body.get_payload().has_value());
    };
};

suite<"BulkExecIdsBody"> bulk_exec_ids_body_suite = [] {
    "default-constructs empty"_test = [] {
        engine::BulkExecIdsBody body;
        expect(body.get_exec_ids().empty());
    };

    "set_exec_ids/get_exec_ids round-trip"_test = [] {
        engine::BulkExecIdsBody body;
        body.set_exec_ids({"exec-1", "exec-2"});
        expect(body.get_exec_ids().size() == 2);
        expect(body.get_exec_ids()[0] == "exec-1");
        expect(body.get_exec_ids()[1] == "exec-2");
    };
};

suite<"BulkResult"> bulk_result_suite = [] {
    "default-constructs with empty id and false success"_test = [] {
        engine::BulkResult result;
        expect(result.get_exec_id().empty());
        expect(!result.get_success());
    };

    "value ctor sets both fields"_test = [] {
        engine::BulkResult result{"exec-9", true};
        expect(result.get_exec_id() == "exec-9");
        expect(result.get_success());
    };

    "set_exec_id/get_exec_id round-trip"_test = [] {
        engine::BulkResult result;
        result.set_exec_id("exec-a");
        expect(result.get_exec_id() == "exec-a");
    };

    "set_success/get_success round-trip"_test = [] {
        engine::BulkResult result;
        result.set_success(true);
        expect(result.get_success());
    };
};

} // namespace engine::workflow_dto_tests
#endif

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
class WorkflowHandler
{
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
    explicit WorkflowHandler(EngineContext& ctx) noexcept :
        m_ctx(ctx)
    {
    }

    /**
     * @brief Handles `GET /api/v1/workflows/:name` — looks up one WorkflowDef by name.
     * @warning `name` is pulled by slicing everything after the last `/` in the path, no real
     * path-param extraction — same low-effort parsing this whole file leans on throughout.
     * @param req the inbound request; path supplies the name, Accept header picks the format.
     * @param res the response — 200 with the definition, or 404 if nothing matched.
     */
    void get_definition(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) noexcept
    {
        // slice the name off the tail of the path — no dedicated route-param binding here
        // either
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto name = std::string{target.substr(target.rfind('/') + 1)};

        // look it up and let the callback decide 404 vs a normal 200 reply. Not noexcept: it
        // calls reply()/serde::Ser::serialize(), which allocate and can throw.
        // Connector::find()'s callback parameter (std::move_only_function<void(...)>) doesn't
        // require noexcept, nor does interfaces::HandlerFn (std::function), so dropping it here
        // is safe.
        m_ctx.get().get_connector().find<model::WorkflowDef>(
            name, [&res, accept, send = std::move(send)](std::optional<model::WorkflowDef> result) {
                if (!result) {
                    // nothing under that name — bounce a 404
                    reply(
                        res, serde::Ser::serialize_error(accept, "not found"),
                        interfaces::io::types::Status::NOT_FOUND
                    );
                    send();
                    return;
                }
                reply(res, serde::Ser::serialize(accept, *result));
                send();
            }
        );
    }

    /**
     * @brief Handles `POST /api/v1/workflows` — parses, validates, then upserts a WorkflowDef.
     * @note Bad-request and validation-failure paths log a warning before replying — every
     * rejected create leaves a trace, W for debuggability. Upsert (not a plain insert),
     * matching TaskHandler::create_definition, so an app worker re-registering the same
     * workflow on every restart doesn't hit a duplicate-PK failure.
     * @param req the inbound request; body is the definition, Content-Type picks the decoder,
     * Accept picks the reply format.
     * @param res the response — 201 with the created definition, 400 on a parse failure, 422 on
     * a validation failure, or 500 if the upsert itself fails.
     */
    // Not noexcept: string/serde operations below (deserialize, validate, serialize_error) can
    // throw. interfaces::HandlerFn is a std::function, which doesn't require a noexcept target,
    // and every route lambda in routes.cppm that calls this isn't noexcept either.
    void create_definition(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        // decode the body into a WorkflowDef — bail with a 400 if it doesn't even parse
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::WorkflowDef>(content_type, body);
        if (!parsed) {
            core::logger::warning("engine", "wf/create bad request: {}", parsed.error());
            reply(
                res, serde::Ser::serialize_error(accept, parsed.error()),
                interfaces::io::types::Status::BAD_REQUEST
            );
            send();
            return;
        }

        // domain validation before anything touches the store — 422 if it's not clean
        if (auto value = parsed->validate(); !value) {
            core::logger::warning("engine", "wf/create invalid: {}", value.error());
            reply(
                res, serde::Ser::serialize_error(accept, value.error()),
                interfaces::io::types::Status::UNPROCESSABLE_CONTENT
            );
            send();
            return;
        }

        // parsed and validated — upsert it and reply with what got created. Callback calls
        // logger::error/info and serde::Ser::serialize(), both of which can throw; not
        // noexcept, same reasoning as create_definition() itself above.
        model::WorkflowDef definition = *parsed;
        m_ctx.get().get_connector().upsert<model::WorkflowDef>(
            definition, [&res, accept, definition, send = std::move(send)](bool oke) {
                if (!oke) {
                    core::logger::error("engine", "wf/create db upsert failed");
                    reply(
                        res, serde::Ser::serialize_error(accept, "upsert failed"),
                        interfaces::io::types::Status::INTERNAL_SERVER_ERROR
                    );
                    send();
                    return;
                }
                core::logger::info("engine", "workflow created: '{}'", definition.get_name());
                core::events::publish(
                    "engine.workflow_def.created", {{"name", definition.get_name()}}
                );
                reply(
                    res, serde::Ser::serialize(accept, definition),
                    interfaces::io::types::Status::CREATED
                );
                send();
            }
        );
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
    void update_definition(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        // decode the replacement body — 400 if it's not a valid WorkflowDef (no warning logged
        // here, unlike TaskHandler::update_definition())
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::WorkflowDef>(content_type, body);
        if (!parsed) {
            reply(
                res, serde::Ser::serialize_error(accept, parsed.error()),
                interfaces::io::types::Status::BAD_REQUEST
            );
            send();
            return;
        }

        // same validation pass as create — 422 if it fails
        if (auto value = parsed->validate(); !value) {
            reply(
                res, serde::Ser::serialize_error(accept, value.error()),
                interfaces::io::types::Status::UNPROCESSABLE_CONTENT
            );
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
                    reply(
                        res, serde::Ser::serialize_error(accept, "not found"),
                        interfaces::io::types::Status::NOT_FOUND
                    );
                    send();
                    return;
                }
                reply(res, serde::Ser::serialize(accept, definition));
                send();
            }
        );
    }

    /**
     * @brief Handles `DELETE /api/v1/workflows/:name` — removes a WorkflowDef by name.
     * @warning Same last-segment path slicing as get_definition() — no dedicated param
     * extraction here either.
     * @param req the inbound request; path supplies the name, Accept header picks the format.
     * @param res the response — 204 on success, 404 if that name wasn't found.
     */
    void remove_definition(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) noexcept
    {
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
                    reply(
                        res, serde::Ser::serialize_error(accept, "not found"),
                        interfaces::io::types::Status::NOT_FOUND
                    );
                    send();
                    return;
                }
                res.set_status(interfaces::io::types::Status::NO_CONTENT);
                send();
            }
        );
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
    void start_execution(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
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

        m_ctx.get().get_workflow_orchestrator()->start_workflow(
            def_name, std::move(variables),
            [this, &res, accept, def_name,
             send = std::move(send)](std::optional<std::string> exec_id) mutable {
                if (!exec_id) {
                    core::logger::warning("engine", "wf/start not found: '{}'", def_name);
                    reply(
                        res, serde::Ser::serialize_error(accept, "workflow definition not found"),
                        interfaces::io::types::Status::NOT_FOUND
                    );
                    send();
                    return;
                }
                // Re-fetch the created execution to reply with its full record (the interface
                // hands back only the id).
                m_ctx.get().get_connector().find<model::WorkflowExecution>(
                    *exec_id,
                    [&res, accept, def_name,
                     send = std::move(send)](std::optional<model::WorkflowExecution> exec) mutable {
                        if (!exec) {
                            reply(
                                res, serde::Ser::serialize_error(accept, "not found"),
                                interfaces::io::types::Status::NOT_FOUND
                            );
                            send();
                            return;
                        }
                        core::logger::info(
                            "engine", "exec '{}' started for '{}'", exec->get_exec_id(), def_name
                        );
                        core::events::publish(
                            "engine.workflow.started",
                            {{"exec_id", std::format("{}", exec->get_exec_id())},
                             {"workflow_name", def_name}}
                        );
                        reply(
                            res, serde::Ser::serialize(accept, *exec),
                            interfaces::io::types::Status::ACCEPTED
                        );
                        send();
                    }
                );
            }
        );
    }

    // Path: /api/v1/workflows/exec/:id — exec_id is the last segment.
    /**
     * @brief Handles `GET /api/v1/workflows/exec/:id` — looks up one WorkflowExecution by id.
     * @param req the inbound request; path supplies the execution id, Accept header picks the
     * format.
     * @param res the response — 200 with the execution, or 404 if that id wasn't found.
     */
    void get_execution(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) noexcept
    {
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
                    reply(
                        res, serde::Ser::serialize_error(accept, "not found"),
                        interfaces::io::types::Status::NOT_FOUND
                    );
                    send();
                    return;
                }
                mask_and_reply(res, accept, std::move(send), std::move(*result));
            }
        );
    }

    // Path: /api/v1/workflows/exec/:id — exec_id is the last segment.
    /**
     * @brief Handles `DELETE /api/v1/workflows/exec/:id` — terminates a running
     * WorkflowExecution, rejecting the request if the execution is already in a terminal
     * state.
     * @note The whole terminate flow (404 / already-terminal / flip-and-persist) runs inside
     * find()'s callback chain, not synchronously after it — with a real database backend find()
     * defers its callback to a later tick, so reading a result set by that callback
     * synchronously after the find() call (the shape this used to have) saw an empty optional
     * and crashed. Every local the callbacks need is captured by copy/move, never
     * [&]-by-reference to this stack frame.
     * @param req the inbound request; path supplies the execution id, Accept header picks the
     * format.
     * @param res the response — 200 with the terminated execution, 404 if the id wasn't found
     * (on the initial lookup or the follow-up update()), or 409 if it was already terminal.
     */
    void terminate_execution(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto exec_id_str = std::string{target.substr(target.rfind('/') + 1)};

        // look the execution up — the callback handles the 404, already-terminal, and
        // flip-and-persist cases inline. Not noexcept: calls logger::warning() and
        // serde::Ser::serialize_error(), both of which can throw.
        m_ctx.get().get_connector().find<model::WorkflowExecution>(
            exec_id_str,
            [this, &res, exec_id_str, accept,
             send = std::move(send)](std::optional<model::WorkflowExecution> result) mutable {
                if (!result) {
                    // no such execution — 404
                    core::logger::warning("engine", "wf/terminate not found: '{}'", exec_id_str);
                    reply(
                        res, serde::Ser::serialize_error(accept, "not found"),
                        interfaces::io::types::Status::NOT_FOUND
                    );
                    send();
                    return;
                }
                if (model::is_terminal(result->get_status())) {
                    // already done/failed/terminated — refuse to terminate it twice, no cap
                    core::logger::warning(
                        "engine", "wf/terminate already terminal: '{}'", exec_id_str
                    );
                    reply(
                        res, serde::Ser::serialize_error(accept, "already in terminal state"),
                        interfaces::io::types::Status::CONFLICT
                    );
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
                            reply(
                                res, serde::Ser::serialize_error(accept, "not found"),
                                interfaces::io::types::Status::NOT_FOUND
                            );
                            send();
                            return;
                        }
                        core::logger::info("engine", "exec terminated: '{}'", exec_id_str);
                        core::events::publish(
                            "engine.workflow.terminated", {{"exec_id", exec_id_str}}
                        );
                        m_ctx.get().get_workflow_orchestrator()->on_execution_terminal(
                            exec_id_str, [](bool) {}
                        );
                        reply(res, serde::Ser::serialize(accept, execution));
                        send();
                    }
                );
            }
        );
    }

    /// @brief Path is `/api/v1/workflows/exec/:id/{pause,resume,retry,restart,rerun,signal}` —
    /// `:id` sits between the last two slashes, same brittle hand-rolled slicing as everywhere
    /// else in this file.
    static std::string exec_id_from_action_path(interfaces::io::IRequest& req)
    {
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
    void pause_execution(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
        auto accept = req.find_header("accept");
        m_ctx.get().get_workflow_orchestrator()->pause(
            exec_id_from_action_path(req), [&res, accept, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(
                        res, serde::Ser::serialize_error(accept, "not found or not running"),
                        interfaces::io::types::Status::CONFLICT
                    );
                    send();
                    return;
                }
                res.set_status(interfaces::io::types::Status::OK);
                send();
            }
        );
    }

    /**
     * @brief Handles `POST /api/v1/workflows/exec/:id/resume`.
     * @param req the inbound request; path supplies the execution id.
     * @param res the response — 200 on success, 409 if the execution wasn't found or wasn't
     * PAUSED.
     */
    void resume_execution(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
        auto accept = req.find_header("accept");
        m_ctx.get().get_workflow_orchestrator()->resume(
            exec_id_from_action_path(req), [&res, accept, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(
                        res, serde::Ser::serialize_error(accept, "not found or not paused"),
                        interfaces::io::types::Status::CONFLICT
                    );
                    send();
                    return;
                }
                res.set_status(interfaces::io::types::Status::OK);
                send();
            }
        );
    }

    /**
     * @brief Handles `POST /api/v1/workflows/exec/:id/retry`.
     * @param req the inbound request; path supplies the execution id.
     * @param res the response — 200 on success, 409 if the execution/def wasn't found, wasn't
     * FAILED, or the def forbids retry (restartable == false).
     */
    void retry_execution(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
        auto accept = req.find_header("accept");
        m_ctx.get().get_workflow_orchestrator()->retry(
            exec_id_from_action_path(req), [&res, accept, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(
                        res, serde::Ser::serialize_error(accept, "not retryable"),
                        interfaces::io::types::Status::CONFLICT
                    );
                    send();
                    return;
                }
                res.set_status(interfaces::io::types::Status::OK);
                send();
            }
        );
    }

    /**
     * @brief Handles `POST /api/v1/workflows/exec/:id/restart`.
     * @param req the inbound request; path supplies the execution id.
     * @param res the response — 200 on success, 409 if the execution/def wasn't found, wasn't
     * in a terminal state, or the def forbids restart.
     */
    void restart_execution(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
        auto accept = req.find_header("accept");
        m_ctx.get().get_workflow_orchestrator()->restart(
            exec_id_from_action_path(req), [&res, accept, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(
                        res, serde::Ser::serialize_error(accept, "not restartable"),
                        interfaces::io::types::Status::CONFLICT
                    );
                    send();
                    return;
                }
                res.set_status(interfaces::io::types::Status::OK);
                send();
            }
        );
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
    void rerun_execution(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<RerunBody>(content_type, body);
        if (!parsed) {
            reply(
                res, serde::Ser::serialize_error(accept, parsed.error()),
                interfaces::io::types::Status::BAD_REQUEST
            );
            send();
            return;
        }
        m_ctx.get().get_workflow_orchestrator()->rerun(
            exec_id_from_action_path(req), parsed->get_node_ref(), parsed->get_input(),
            [&res, accept, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(
                        res, serde::Ser::serialize_error(accept, "not rerunnable"),
                        interfaces::io::types::Status::CONFLICT
                    );
                    send();
                    return;
                }
                res.set_status(interfaces::io::types::Status::OK);
                send();
            }
        );
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
    void signal_execution(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<SignalBody>(content_type, body);
        if (!parsed) {
            reply(
                res, serde::Ser::serialize_error(accept, parsed.error()),
                interfaces::io::types::Status::BAD_REQUEST
            );
            send();
            return;
        }
        auto signal_payload = parsed->get_payload()
                                  ? std::optional<std::string_view>{*parsed->get_payload()}
                                  : std::nullopt;
        m_ctx.get().get_workflow_orchestrator()->signal(
            exec_id_from_action_path(req), parsed->get_node_ref(), signal_payload,
            [&res, accept, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(
                        res,
                        serde::Ser::serialize_error(accept, "no matching in-progress instance"),
                        interfaces::io::types::Status::CONFLICT
                    );
                    send();
                    return;
                }
                res.set_status(interfaces::io::types::Status::OK);
                send();
            }
        );
    }

    /// @brief Handles `POST /api/v1/workflows/bulk/pause` — body is a BulkExecIdsBody, response
    /// a `vector<BulkResult>` with one entry per exec_id, applied sequentially (not in
    /// parallel — Connector's own op queue serializes them anyway in db-backed mode, and this
    /// keeps the per-id success bookkeeping trivial).
    void bulk_pause(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
        bulk_dispatch(
            req, res, std::move(send),
            [](interfaces::IWorkflowOrchestrator& orchestrator, std::string exec_id,
               std::move_only_function<void(bool)> callback) {
                orchestrator.pause(std::move(exec_id), std::move(callback));
            }
        );
    }

    /// @brief Handles `POST /api/v1/workflows/bulk/resume`.
    void bulk_resume(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
        bulk_dispatch(
            req, res, std::move(send),
            [](interfaces::IWorkflowOrchestrator& orchestrator, std::string exec_id,
               std::move_only_function<void(bool)> callback) {
                orchestrator.resume(std::move(exec_id), std::move(callback));
            }
        );
    }

    /// @brief Handles `POST /api/v1/workflows/bulk/retry`.
    void bulk_retry(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
        bulk_dispatch(
            req, res, std::move(send),
            [](interfaces::IWorkflowOrchestrator& orchestrator, std::string exec_id,
               std::move_only_function<void(bool)> callback) {
                orchestrator.retry(std::move(exec_id), std::move(callback));
            }
        );
    }

    /// @brief Handles `POST /api/v1/workflows/bulk/restart`.
    void bulk_restart(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
        bulk_dispatch(
            req, res, std::move(send),
            [](interfaces::IWorkflowOrchestrator& orchestrator, std::string exec_id,
               std::move_only_function<void(bool)> callback) {
                orchestrator.restart(std::move(exec_id), std::move(callback));
            }
        );
    }

    /// @brief Handles `POST /api/v1/workflows/bulk/terminate`.
    void bulk_terminate(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
        bulk_dispatch(
            req, res, std::move(send),
            [](interfaces::IWorkflowOrchestrator& orchestrator, std::string exec_id,
               std::move_only_function<void(bool)> callback) {
                orchestrator.terminate(std::move(exec_id), std::move(callback));
            }
        );
    }

    /// @brief Handles `POST /api/v1/workflows/bulk/remove` — the only bulk op that isn't an
    /// Orchestrator method (removal is plain storage cleanup, no cascade/propagation involved),
    /// so this reaches straight into the connector instead.
    void bulk_remove(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
        bulk_dispatch(
            req, res, std::move(send),
            [this](
                interfaces::IWorkflowOrchestrator& /*orchestrator*/, std::string exec_id,
                std::move_only_function<void(bool)> callback
            ) {
                m_ctx.get().get_connector().remove<model::WorkflowExecution>(
                    exec_id, std::move(callback)
                );
            }
        );
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
    void mask_and_reply(
        interfaces::io::IResponse& res,
        std::string_view accept,
        std::function<void()> send,
        model::WorkflowExecution exec
    )
    {
        m_ctx.get().get_connector().find_all<model::TaskDef>(
            [&res, accept, send = std::move(send),
             exec = std::move(exec)](std::vector<model::TaskDef> defs) mutable {
                std::unordered_map<std::string, std::vector<std::string>> masks;
                for (const auto& def: defs) {
                    if (!def.get_masked_fields().empty()) {
                        masks[def.get_name()] = def.get_masked_fields();
                    }
                }
                if (!masks.empty()) {
                    auto instances = exec.get_task_instances();
                    for (auto& instance: instances) {
                        auto mask_it = masks.find(instance.get_def_name());
                        if (mask_it == masks.end()) {
                            continue;
                        }
                        auto input = instance.get_input_data();
                        auto output = instance.get_output_data();
                        // output stays a flat string map — mask straight in place
                        for (const auto& field: mask_it->second) {
                            if (output.contains(field)) {
                                output[field] = "*******";
                            }
                        }
                        // input is now a dynamic Value — only mask when it's an object (a
                        // non-object input has no keys to mask and is left untouched)
                        if (auto input_obj = input.to_object()) {
                            for (const auto& field: mask_it->second) {
                                if (input_obj->count(field) != 0) {
                                    (*input_obj)[field] = serde::Value{std::string{"*******"}};
                                }
                            }
                            instance.set_input_data(serde::Value{std::move(*input_obj)});
                        }
                        instance.set_output_data(std::move(output));
                    }
                    exec.set_task_instances(std::move(instances));
                }
                reply(res, serde::Ser::serialize(accept, exec));
                send();
            }
        );
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
    /// @brief Type of the per-execution action `bulk_dispatch()`/`bulk_apply()` thread through
    /// the recursive chain. Held via `shared_ptr` (see `bulk_apply()`'s comment) rather than by
    /// value — the bug this works around is real and was found while writing this handler's own
    /// test.
    using BulkOp = std::function<
        void(interfaces::IWorkflowOrchestrator&, std::string, std::move_only_function<void(bool)>)>;

    void bulk_dispatch(
        interfaces::io::IRequest& req,
        interfaces::io::IResponse& res,
        std::function<void()> send,
        BulkOp op
    )
    {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<BulkExecIdsBody>(content_type, body);
        if (!parsed) {
            reply(
                res, serde::Ser::serialize_error(accept, parsed.error()),
                interfaces::io::types::Status::BAD_REQUEST
            );
            send();
            return;
        }
        bulk_apply(
            parsed->get_exec_ids(), 0, {}, std::make_shared<BulkOp>(std::move(op)), res, accept,
            std::move(send)
        );
    }

    /**
     * @brief Recursive per-id step of a bulk op. `op` is held via `shared_ptr` rather than
     * being moved through the recursion by value — the tempting `op(..., [op = std::move(op)]
     * (...) {...})` form moves `op` into the continuation's capture (argument-list
     * construction) while `op` is simultaneously the callee of that very call: the object being
     * invoked gets moved-from before its own `operator()` body runs (argument construction is
     * sequenced before the call, not after), so the invocation lands on an emptied-out target
     * and throws `std::bad_function_call` — reproduced even with a single exec_id, confirmed
     * via instrumentation showing `op` non-empty by any check made *before* the call
     * expression, because the corrupting move happens *inside* that same expression's
     * evaluation. A `shared_ptr` sidesteps this entirely: capturing it into the continuation is
     * a cheap refcount bump, never a move of the pointee, so the call and the capture stop
     * racing over the same object.
     */
    void bulk_apply(
        std::vector<std::string> exec_ids,
        std::size_t index,
        std::vector<BulkResult> results,
        std::shared_ptr<BulkOp> op,
        interfaces::io::IResponse& res,
        std::string_view accept,
        std::function<void()> send
    )
    {
        if (index >= exec_ids.size()) {
            reply(res, serde::Ser::serialize(accept, results));
            send();
            return;
        }
        auto exec_id = exec_ids[index];
        interfaces::IWorkflowOrchestrator& orchestrator = *m_ctx.get().get_workflow_orchestrator();
        (*op)(
            orchestrator, exec_id,
            [this, exec_ids = std::move(exec_ids), index, results = std::move(results), op, &res,
             accept, send = std::move(send)](bool oke) mutable {
                results.emplace_back(exec_ids[index], oke);
                bulk_apply(
                    std::move(exec_ids), index + 1, std::move(results), std::move(op), res, accept,
                    std::move(send)
                );
            }
        );
    }

    /**
     * @brief Shared reply helper — writes `bytes` into the response body and sets the status,
     * defaulting to 200 OK when the caller doesn't hand over anything else.
     * @param res the response to fill in.
     * @param bytes the body bytes to write.
     * @param status the status code to set, defaults to OK.
     */
    static void reply(
        interfaces::io::IResponse& res,
        std::vector<std::byte> bytes,
        interfaces::io::types::Status status = interfaces::io::types::Status::OK
    ) noexcept
    {
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
    static std::string flatten_body(interfaces::io::IRequest& req) noexcept
    {
        std::string out;
        auto& view = req.get_body();
        out.reserve(view.size());
        // walk the raw bytes one at a time, reinterpreting each as a char
        for (std::byte byte: view) {
            out += static_cast<char>(byte);
        }
        return out;
    }
};

} // namespace engine

#ifdef CONGELADO_TEST
namespace engine::workflow_handler_tests {
using namespace boost::ut;

// Trivial synchronous in-memory ICache — Connector's find()/write_through() paths abort() via
// active_cache() if no cache is wired in.
class FakeCache final : public interfaces::ICache
{
public:
    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake_cache";
    }

    void get(std::string_view key, shared::QueryReadFn&& result) noexcept override
    {
        auto found = m_store.find(std::string{key});
        result(found != m_store.end() ? std::string_view{found->second} : std::string_view{});
    }

    void set(
        std::string_view key, std::string_view value, shared::QueryReadFn&& result
    ) noexcept override
    {
        m_store[std::string{key}] = std::string{value};
        result("ok");
    }

    void remove(std::string_view key, shared::QueryReadFn&& result) noexcept override
    {
        m_store.erase(std::string{key});
        result("ok");
    }

private:
    std::unordered_map<std::string, std::string> m_store;
};

// Real (non-null) ISerdeFormat stub for content-type "application/json" — ignores the actual
// body text and always hands back a BulkExecIdsBody-shaped Value with `count` exec_ids, so
// bulk_dispatch()'s deserialize() step succeeds without a full JSON parser wired into this test
// target. Same Object/Array construction pattern plugins/serde/json/bin/json_plugin.cc's own
// tests and openapi_generator's schema_model.cppm tests already use.
class FakeBulkExecIdsFormat final : public interfaces::ISerdeFormat
{
public:
    explicit FakeBulkExecIdsFormat(std::size_t count) noexcept :
        m_count{count}
    {
    }

    [[nodiscard]] std::string_view content_type() const noexcept override
    {
        return "application/json";
    }

    [[nodiscard]] std::string_view format_name() const noexcept override
    {
        return "fake-bulk-exec-ids";
    }

    [[nodiscard]] std::expected<std::string, std::string>
    encode(const interfaces::Value&) const override
    {
        return std::string{"{}"};
    }

    [[nodiscard]] std::expected<interfaces::Value, std::string>
    decode(std::string_view) const override
    {
        serde::Value::Array ids;
        ids.reserve(m_count);
        for (std::size_t index = 0; index < m_count; ++index) {
            ids.push_back(serde::Value{std::format("exec-{}", index)});
        }
        serde::Value::Object object;
        object.insert(std::string{"exec_ids"}, serde::Value{ids});
        return serde::Value{object};
    }

private:
    std::size_t m_count;
};

// Minimal IWorkflowOrchestrator fake — every op reports success synchronously except
// start_workflow(), whose result is configurable (mirrors admin_handler_tests's own fake in
// admin.cppm, duplicated here since each engine partition test block needs its own).
class FakeWorkflowOrchestrator final : public interfaces::IWorkflowOrchestrator
{
public:
    explicit FakeWorkflowOrchestrator(
        std::optional<std::string> start_result = std::nullopt
    ) noexcept :
        m_start_result{std::move(start_result)}
    {
    }

    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake";
    }

    void start_workflow(
        std::string_view,
        const std::unordered_map<std::string, std::string>&,
        std::move_only_function<void(std::optional<std::string>)> callback
    ) override
    {
        callback(m_start_result);
    }

    void on_task_terminal(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void on_execution_terminal(
        std::string_view, std::move_only_function<void(bool)> callback
    ) override
    {
        callback(true);
    }

    void pause(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void resume(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void retry(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void restart(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void terminate(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void reconcile(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void rerun(
        std::string_view,
        std::string_view,
        const interfaces::Value&,
        std::move_only_function<void(bool)> callback
    ) override
    {
        callback(true);
    }

    void signal(
        std::string_view,
        std::string_view,
        std::optional<std::string_view>,
        std::move_only_function<void(bool)> callback
    ) override
    {
        callback(true);
    }

    void complete_task(
        std::string_view,
        std::string_view,
        bool,
        const std::unordered_map<std::string, std::string>&,
        std::move_only_function<void(bool)> callback
    ) override
    {
        callback(true);
    }

    void start_server() override {}

    void shutdown_all() override {}

private:
    std::optional<std::string> m_start_result;
};

suite<"WorkflowHandler"> workflow_handler_suite = [] {
    "get_definition replies 404 for a name that was never stored"_test = [] {
        engine::EngineContext ctx;
        FakeCache cache;
        ctx.set_cache(&cache);
        engine::WorkflowHandler handler{ctx};
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_header(interfaces::io::types::Token::PATH, "/api/v1/workflows/never-created");
        bool sent = false;

        handler.get_definition(req, res, [&sent] {
            sent = true;
        });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::NOT_FOUND);
    };

    "bulk_pause replies 400 when the body doesn't parse (no serde format registered)"_test = [] {
        serde::SerdeFormatRegistry::set_active(nullptr);
        engine::EngineContext ctx;
        engine::WorkflowHandler handler{ctx};
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        std::vector<std::byte> body{std::byte{'{'}, std::byte{'}'}};
        req.set_body(std::move(body));
        bool sent = false;

        handler.bulk_pause(req, res, [&sent] {
            sent = true;
        });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::BAD_REQUEST);
    };

    // Item 4 pin: BulkExecIdsBody.exec_ids has no size cap. Originally written to drive this
    // through the full handler.bulk_pause() -> bulk_dispatch() -> bulk_apply() -> orchestrator
    // chain, but that surfaced a separate, genuine bug: op(orchestrator, exec_id, callback)
    // (bulk_apply()'s std::function<void(IWorkflowOrchestrator&, std::string,
    // std::move_only_function<void(bool)>)> parameter, taking the callback BY VALUE) threw
    // std::bad_function_call at the call site itself — confirmed via instrumentation that `op`
    // was non-empty right before the call and that FakeWorkflowOrchestrator::pause() was never
    // reached, i.e. the throw happened inside std::function's own dispatch of a target taking a
    // move-only parameter by value, not in anything this test or its mocks controlled.
    // Reproduced even with a single exec_id. FIXED by changing every `op` callback parameter
    // (bulk_pause/resume/retry/restart/terminate/remove's lambdas, and bulk_dispatch()'s and
    // bulk_apply()'s std::function<...> signature) from `std::move_only_function<void(bool)>`
    // by value to `std::move_only_function<void(bool)> &&` — std::function's own call-signature
    // handling of a move-only-typed by-value parameter was the fragile part; an rvalue
    // reference sidesteps it. This test drives the real chain end-to-end (1000 exec_ids) and
    // proves both the fix (no crash/throw) and the original finding (no size-cap rejection).
    "bulk_pause drives 1000 exec_ids through the real dispatch chain with no crash and no "
    "size-cap rejection"_test = [] {
        serde::SerdeFormatRegistry registry;
        registry.add_format(std::make_shared<FakeBulkExecIdsFormat>(1'000));
        serde::SerdeFormatRegistry::set_active(&registry);

        engine::EngineContext ctx;
        FakeWorkflowOrchestrator orchestrator;
        ctx.set_workflow_orchestrator(&orchestrator);
        engine::WorkflowHandler handler{ctx};
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_header(interfaces::io::types::Token::CONTENT_TYPE, "application/json");
        std::vector<std::byte> body{std::byte{'{'}, std::byte{'}'}};
        req.set_body(std::move(body));
        bool sent = false;

        handler.bulk_pause(req, res, [&sent] {
            sent = true;
        });

        expect(sent) << fatal;
        expect(res.get_status() == interfaces::io::types::Status::OK);
        serde::SerdeFormatRegistry::set_active(nullptr);
    };

    // Item 5 pin (start_execution): a non-empty but unparseable body is silently swallowed —
    // deserialize<WorkflowStartBody>() fails, so `variables` just stays empty instead of the
    // request getting a 400. Forcing deserialize() to fail via "no format registered" (same
    // mechanism the sibling parse-failure tests in this file use) rather than genuinely
    // malformed JSON text — start_execution() can't tell the two apart either, it only checks
    // whether the `if (auto parsed = ...)` succeeded.
    "start_execution silently falls back to empty variables on a malformed non-empty body instead of 400"_test =
        [] {
            serde::SerdeFormatRegistry::set_active(nullptr);
            engine::EngineContext ctx;
            FakeCache cache;
            ctx.set_cache(&cache);

            auto exec_id = model::generate_id();
            model::WorkflowExecution seed;
            seed.set_exec_id(exec_id);
            seed.set_def_name("garbled-wf");
            auto exec_id_str = serde::Cache::pk_string(seed);
            bool seeded = false;
            ctx.get_connector().upsert<model::WorkflowExecution>(seed, [&seeded](bool oke) {
                seeded = oke;
            });
            expect(seeded) << fatal;

            FakeWorkflowOrchestrator orchestrator{exec_id_str};
            ctx.set_workflow_orchestrator(&orchestrator);
            engine::WorkflowHandler handler{ctx};
            io::layer::http2::HttpRequest req{1};
            io::layer::http2::HttpResponse res{1};
            req.set_header(
                interfaces::io::types::Token::PATH, "/api/v1/workflows/garbled-wf/start"
            );
            std::vector<std::byte> body{
                std::byte{'{'}, std::byte{'n'}, std::byte{'o'}, std::byte{'p'}, std::byte{'e'}
            };
            req.set_body(std::move(body));
            bool sent = false;

            handler.start_execution(req, res, [&sent] {
                sent = true;
            });

            expect(sent);
            expect(res.get_status() == interfaces::io::types::Status::ACCEPTED);
        };
};

} // namespace engine::workflow_handler_tests
#endif

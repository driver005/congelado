export module engine:workflow;

import std;
import interfaces;
import model;
import shared;
import serde;
import core_logger;
import :context;

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
    void get_definition(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        // slice the name off the tail of the path — no dedicated route-param binding here either
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto name = std::string{target.substr(target.rfind('/') + 1)};

        // look it up and let the callback decide 404 vs a normal 200 reply. Not noexcept: it
        // calls reply()/serde::Ser::serialize(), which allocate and can throw. Connector::find()'s
        // callback parameter (std::move_only_function<void(...)>) doesn't require noexcept, nor
        // does interfaces::HandlerFn (std::function), so dropping it here is safe.
        m_ctx.get().get_connector().find<model::WorkflowDef>(
            name, [&](std::optional<model::WorkflowDef> result) {
                if (!result) {
                    // nothing under that name — bounce a 404
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    return;
                }
                reply(res, serde::Ser::serialize(accept, *result));
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
    void create_definition(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        // decode the body into a WorkflowDef — bail with a 400 if it doesn't even parse
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::WorkflowDef>(content_type, body);
        if (!parsed) {
            core::logger::warning("engine", "wf/create bad request: {}", parsed.error());
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            return;
        }

        // domain validation before anything touches the store — 422 if it's not clean
        if (auto value = parsed->validate(); !value) {
            core::logger::warning("engine", "wf/create invalid: {}", value.error());
            reply(res, serde::Ser::serialize_error(accept, value.error()),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            return;
        }

        // parsed and validated — insert it and reply with what got created. Callback calls
        // logger::error/info and serde::Ser::serialize(), both of which can throw; not
        // noexcept, same reasoning as create_definition() itself above.
        m_ctx.get().get_connector().insert<model::WorkflowDef>(*parsed, [&](bool oke) {
            if (!oke) {
                core::logger::error("engine", "wf/create db insert failed");
                reply(res, serde::Ser::serialize_error(accept, "insert failed"),
                      interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
                return;
            }
            core::logger::info("engine", "workflow created: '{}'", parsed->get_name());
            reply(res, serde::Ser::serialize(accept, *parsed),
                  interfaces::io::types::Status::CREATED);
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
    void update_definition(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        // decode the replacement body — 400 if it's not a valid WorkflowDef (no warning logged here,
        // unlike TaskHandler::update_definition())
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::WorkflowDef>(content_type, body);
        if (!parsed) {
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            return;
        }

        // same validation pass as create — 422 if it fails
        if (auto value = parsed->validate(); !value) {
            reply(res, serde::Ser::serialize_error(accept, value.error()),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            return;
        }

        // key off the name in the body, not the URL segment — update() 404s if it's not there.
        // Callback calls serde::Ser::serialize()/serialize_error(), which can throw; not
        // noexcept, same reasoning as the insert() callback above.
        m_ctx.get().get_connector().update<model::WorkflowDef>(*parsed, [&](bool oke) {
            if (!oke) {
                reply(res, serde::Ser::serialize_error(accept, "not found"),
                      interfaces::io::types::Status::NOT_FOUND);
                return;
            }
            reply(res, serde::Ser::serialize(accept, *parsed));
        });
    }

    /**
     * @brief Handles `DELETE /api/v1/workflows/:name` — removes a WorkflowDef by name.
     * @warning Same last-segment path slicing as get_definition() — no dedicated param
     * extraction here either.
     * @param req the inbound request; path supplies the name, Accept header picks the format.
     * @param res the response — 204 on success, 404 if that name wasn't found.
     */
    void remove_definition(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        // same tail-slicing move as get_definition to pull the name back out
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto name = std::string{target.substr(target.rfind('/') + 1)};

        // delete by name — 404 if there was nothing to delete, else 204. Callback calls
        // serde::Ser::serialize_error(), which can throw; not noexcept, same reasoning as the
        // other callbacks in this class.
        m_ctx.get().get_connector().remove<model::WorkflowDef>(name, [&](bool oke) {
            if (!oke) {
                reply(res, serde::Ser::serialize_error(accept, "not found"),
                      interfaces::io::types::Status::NOT_FOUND);
                return;
            }
            res.set_status(interfaces::io::types::Status::NO_CONTENT);
        });
    }

    // Path: /api/v1/workflows/:name/start — def_name is the segment before "start".
    /**
     * @brief Handles `POST /api/v1/workflows/:name/start` — spins up a new RUNNING
     * WorkflowExecution against `:name`, with optional variable bindings from an optional JSON
     * body, and stamps its start time.
     * @warning `def_name` is carved out by hand via two `rfind('/')` calls, same brittle
     * pattern as TaskHandler::enqueue_task()/submit_result().
     * @warning A non-empty but malformed body gets swallowed silently — if deserialize() fails,
     * `variables` just stays empty instead of the request getting a 400. Same body-swallowing
     * move as TaskHandler::enqueue_task().
     * @note Unlike TaskHandler::enqueue_task(), there's no existence check on the named
     * WorkflowDef before inserting the execution — this fires straight into insert() with
     * whatever `def_name` it parsed, no 404 path for a bogus name up front.
     * @param req the inbound request; path supplies the definition name, an optional JSON body
     * supplies variable bindings, Accept picks the reply format.
     * @param res the response — 202 with the new execution, or 500 if the insert fails.
     */
    // Not noexcept — model construction, serde::Ser::deserialize()/serialize(), and
    // std::chrono formatting below can throw. Same reasoning as create_definition() above.
    void start_execution(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
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

        // no existence check on def_name here — straight into building a fresh RUNNING execution
        model::WorkflowExecution exec;
        exec.set_exec_id(model::generate_id());
        exec.set_def_name(def_name);
        exec.set_status(model::WorkflowStatus::RUNNING);
        exec.set_variables(std::move(variables));

        // stamp the start time before it ever hits the store
        model::ExecutionTimings timings;
        timings.set_started_at(std::chrono::system_clock::now());
        exec.set_timings(timings);

        // Callback calls logger::error/info and serde::Ser::serialize(), both of which can
        // throw; not noexcept, same reasoning as create_definition()'s insert callback above.
        m_ctx.get().get_connector().insert<model::WorkflowExecution>(exec, [&](bool oke) {
            if (!oke) {
                core::logger::error("engine", "wf/start insert failed for '{}'", def_name);
                reply(res, serde::Ser::serialize_error(accept, "insert failed"),
                      interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
                return;
            }
            core::logger::info("engine", "exec '{}' started for '{}'", exec.get_exec_id(),
                               def_name);
            reply(res, serde::Ser::serialize(accept, exec),
                  interfaces::io::types::Status::ACCEPTED);
        });
    }

    // Path: /api/v1/workflows/exec/:id — exec_id is the last segment.
    /**
     * @brief Handles `GET /api/v1/workflows/exec/:id` — looks up one WorkflowExecution by id.
     * @param req the inbound request; path supplies the execution id, Accept header picks the
     * format.
     * @param res the response — 200 with the execution, or 404 if that id wasn't found.
     */
    void get_execution(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        // exec_id is the last path segment
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto exec_id_str = std::string{target.substr(target.rfind('/') + 1)};

        // look it up and let the callback decide 404 vs 200. Not noexcept — same reasoning as
        // get_definition()'s find() callback above.
        m_ctx.get().get_connector().find<model::WorkflowExecution>(
            exec_id_str, [&](std::optional<model::WorkflowExecution> result) {
                if (!result) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    return;
                }
                reply(res, serde::Ser::serialize(accept, *result));
            });
    }

    // Path: /api/v1/workflows/exec/:id — exec_id is the last segment.
    /**
     * @brief Handles `DELETE /api/v1/workflows/exec/:id` — terminates a running
     * WorkflowExecution, rejecting the request if the execution is already in a terminal
     * state.
     * @warning `handled`/`found` are read immediately after the find() call returns, and
     * `found->set_status(...)` runs unconditionally once `!handled` — this whole flow assumes
     * find()'s callback already ran synchronously by that point. That's only true when no
     * database is configured (see the constructor docs above for the full deferred-callback
     * story). With a real database wired in, find() defers its callback to a later tick: this
     * function would then read `handled == false` and `found == std::nullopt` before the real
     * callback ever fires, and `found->set_status(...)` dereferences a `nullopt` optional — a
     * straight-up crash/UB, not a maybe. This is the sharpest edge of the whole class's
     * deferred-callback issue, worth fixing before a real db backend ever gets pointed at this.
     * @param req the inbound request; path supplies the execution id, Accept header picks the
     * format.
     * @param res the response — 200 with the terminated execution, 404 if the id wasn't found
     * (on the initial lookup or the follow-up update()), or 409 if it was already terminal.
     */
    void terminate_execution(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto exec_id_str = std::string{target.substr(target.rfind('/') + 1)};

        std::optional<model::WorkflowExecution> found;
        bool handled = false;
        // look the execution up — the callback below handles the 404 and already-terminal cases
        // inline. Not noexcept: calls logger::warning() and serde::Ser::serialize_error(), both
        // of which can throw.
        m_ctx.get().get_connector().find<model::WorkflowExecution>(
            exec_id_str, [&](std::optional<model::WorkflowExecution> result) {
                if (!result) {
                    // no such execution — 404 and mark handled so the code below skips the update
                    core::logger::warning("engine", "wf/terminate not found: '{}'", exec_id_str);
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    handled = true;
                    return;
                }
                if (model::is_terminal(result->get_status())) {
                    // already done/failed/terminated — refuse to terminate it twice, no cap
                    core::logger::warning("engine", "wf/terminate already terminal: '{}'",
                                          exec_id_str);
                    reply(res, serde::Ser::serialize_error(accept, "already in terminal state"),
                          interfaces::io::types::Status::CONFLICT);
                    handled = true;
                    return;
                }
                // still running — stash it so the code below can flip its status
                found = std::move(result);
            });

        // bail if the callback above already replied (not found or already terminal)
        if (handled) {
            return;
        }

        // flip to TERMINATED and persist it. `found` is only guaranteed set when find()'s
        // callback ran synchronously (local/no-db mode); with a real database wired in this is
        // read before the deferred callback ever fires — see the deferred-callback warning
        // documented on this method above. That's a genuine pre-existing architectural gap, not
        // something a local guard here can safely paper over — so .value() is used (rather than
        // -> / *) to turn the misuse into a defined std::bad_optional_access throw instead of UB.
        found.value().set_status(model::WorkflowStatus::TERMINATED);  // NOLINT(bugprone-unchecked-optional-access) — .value() is the deliberate check, throws instead of UB by design
        // Callback calls logger::info() and serde::Ser::serialize(), both of which can throw;
        // not noexcept, same reasoning as the other connector callbacks in this class.
        m_ctx.get().get_connector().update<model::WorkflowExecution>(
            found.value(),  // NOLINT(bugprone-unchecked-optional-access) — same deliberate .value() check
            [&](bool oke) {
                if (!oke) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    return;
                }
                core::logger::info("engine", "exec terminated: '{}'", exec_id_str);
                reply(res, serde::Ser::serialize(accept, found.value()));
            });
    }

  private:
    std::reference_wrapper<EngineContext> m_ctx;

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

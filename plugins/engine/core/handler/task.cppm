export module engine:task;

import std;
import interfaces;
import model;
import shared;
import serde;
import core_logger;
import :context;

namespace engine {

class TaskSubmitBody {
  public:
    /**
     * @brief Sets the result the worker is reporting.
     * @param result the outcome to record.
     */
    void set_result(model::TaskResult result) noexcept { m_result = result; }
    /**
     * @brief Sets the output data the worker produced.
     * @param data the output key/value pairs to store, moved in.
     */
    void set_output_data(std::unordered_map<std::string, std::string> data) noexcept {
        m_output_data = std::move(data);
    }
    /**
     * @brief Gets the recorded result.
     * @return the outcome, defaults to SUCCESS if never set.
     */
    [[nodiscard]] model::TaskResult get_result() const noexcept { return m_result; }
    /**
     * @brief Gets the recorded output data.
     * @return the output key/value pairs.
     */
    [[nodiscard]] const std::unordered_map<std::string, std::string> &
    get_output_data() const noexcept {
        return m_output_data;
    }

  private:
    model::TaskResult m_result{model::TaskResult::SUCCESS};
    std::unordered_map<std::string, std::string> m_output_data;
};

class TaskEnqueueBody {
  public:
    /**
     * @brief Sets the input data to hand the worker once it claims this instance.
     * @param data the input key/value pairs to store, moved in.
     */
    void set_input_data(std::unordered_map<std::string, std::string> data) noexcept {
        m_input_data = std::move(data);
    }
    /**
     * @brief Sets the ordering sequence number for this enqueue request.
     * @param seq the sequence value — poll() sorts scheduled instances by this.
     */
    void set_seq(std::uint32_t seq) noexcept { m_seq = seq; }
    /**
     * @brief Gets the recorded input data.
     * @return the input key/value pairs.
     */
    [[nodiscard]] const std::unordered_map<std::string, std::string> &
    get_input_data() const noexcept {
        return m_input_data;
    }
    /**
     * @brief Gets the recorded sequence number.
     * @return the sequence value, defaults to 0 if never set.
     */
    [[nodiscard]] std::uint32_t get_seq() const noexcept { return m_seq; }

  private:
    std::unordered_map<std::string, std::string> m_input_data;
    std::uint32_t m_seq{0};
};

} // namespace engine

template <>
struct serde::Serializable<engine::TaskSubmitBody> {
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"result", &engine::TaskSubmitBody::get_result,
                             &engine::TaskSubmitBody::set_result>{},
            serde::FieldDesc<"output_data", &engine::TaskSubmitBody::get_output_data,
                             &engine::TaskSubmitBody::set_output_data>{},
        };
    }
};

template <>
struct serde::Serializable<engine::TaskEnqueueBody> {
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"input_data", &engine::TaskEnqueueBody::get_input_data,
                             &engine::TaskEnqueueBody::set_input_data>{},
            serde::FieldDesc<"seq", &engine::TaskEnqueueBody::get_seq,
                             &engine::TaskEnqueueBody::set_seq>{},
        };
    }
};


export namespace engine {

// Routes:
//   GET    /api/v1/tasks/:name              → get_definition
//   POST   /api/v1/tasks                    → create_definition
//   PUT    /api/v1/tasks/:name              → update_definition
//   DELETE /api/v1/tasks/:name              → remove_definition
//   GET    /api/v1/tasks/queue/:type        → poll
//   POST   /api/v1/tasks/:name/enqueue      → enqueue_task
//   POST   /api/v1/tasks/:id/result         → submit_result
class TaskHandler {
  public:
    /**
     * @brief Builds a handler bound to the shared EngineContext — no state of its own, every
     * route below leans on `m_ctx` to reach the connector.
     * @warning Every route method below captures its local `req`/`res`-derived variables
     * (`accept`, `target`, `name`, `content_type`, ...) by reference (`[&]`) into the callback
     * it hands to Connector::find/insert/update/remove(). Per Connector::enqueue()'s own docs,
     * that callback runs synchronously only when no database is configured — with a real
     * database wired in, the op gets queued and only drained on a later controller tick, by
     * which point the handler method has already returned and those captured locals are
     * dangling stack references. `req`/`res` themselves stay safe (capturing a reference
     * parameter binds straight to the referent, not the local reference variable), but
     * everything else derived from them on the stack does not. This is a latent UB risk across
     * this whole class the moment a database backend gets set — not touching it here since this
     * pass is comment-only, but heads up.
     * @param ctx the engine context to bind; caller keeps it alive for this handler's whole
     * lifetime.
     */
    explicit TaskHandler(EngineContext &ctx) noexcept : m_ctx{ctx} {}

    /**
     * @brief Handles `GET /api/v1/tasks/:name` — looks up one TaskDef by name.
     * @warning `name` is pulled by slicing everything after the last `/` in the path, no real
     * path-param extraction — same low-effort parsing this whole file leans on throughout.
     * @param req the inbound request; path supplies the name, Accept header picks the format.
     * @param res the response — 200 with the definition, or 404 if nothing matched.
     */
    void get_definition(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        // slice the name off the tail of the path — no dedicated route-param binding here
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto name = std::string{target.substr(target.rfind('/') + 1)};

        // look it up and let the callback decide 404 vs a normal 200 reply. Not noexcept: the
        // callback formats/serializes the reply (serde::Ser::serialize*, logger), which may
        // throw, and Connector::find()'s callback parameter (std::move_only_function<void(...)>)
        // doesn't require noexcept — nor does HandlerFn (std::function), so this is safe.
        m_ctx.get().get_connector().find<model::TaskDef>(
            name, [&](std::optional<model::TaskDef> result) {
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
     * @brief Handles `POST /api/v1/tasks` — parses, validates, then inserts a new TaskDef.
     * @note Bad-request and validation-failure paths both log a warning before replying, so
     * every rejected create attempt leaves a trace — good motion for debugging client L's after
     * the fact.
     * @param req the inbound request; body is the definition, Content-Type picks the decoder,
     * Accept picks the reply format.
     * @param res the response — 201 with the created definition, 400 on a parse failure, 422 on
     * a validation failure, or 500 if the insert itself fails.
     */
    // Not noexcept: body does string/JSON parsing, validation, and logging, any of which may
    // throw. HandlerFn (interfaces::HandlerFn = std::function<void(IRequest&, IResponse&)>)
    // doesn't require a noexcept target, and every route lambda in routes.cppm that calls this
    // isn't noexcept either, so dropping it here is safe.
    void create_definition(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        // decode the body into a TaskDef — bail with a 400 if it doesn't even parse
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::TaskDef>(content_type, body);
        if (!parsed) {
            core::logger::warning("engine", "task/create bad request: {}", parsed.error());
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            return;
        }

        // run domain validation before it ever touches the store — 422 if it's not clean
        if (auto validate = parsed->validate(); !validate) {
            core::logger::warning("engine", "task/create invalid: {}", validate.error());
            reply(res, serde::Ser::serialize_error(accept, validate.error()),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            return;
        }

        // parsed and validated — insert it and reply with what got created. Callback logs and
        // serializes the reply (may throw); not noexcept, same reasoning as find()'s above.
        m_ctx.get().get_connector().insert<model::TaskDef>(*parsed, [&](bool oke) {
            if (!oke) {
                core::logger::error("engine", "task/create db insert failed");
                reply(res, serde::Ser::serialize_error(accept, "insert failed"),
                      interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
                return;
            }
            core::logger::info("engine", "task created: '{}'", parsed->get_name());
            reply(res, serde::Ser::serialize(accept, *parsed),
                  interfaces::io::types::Status::CREATED);
        });
    }

    /**
     * @brief Handles `PUT /api/v1/tasks/:name` — parses, validates, then updates an existing
     * TaskDef.
     * @note `parsed->get_name()` from the decoded body is what actually gets used as the
     * update key, not the `:name` path segment — a body whose name doesn't match the URL just
     * quietly updates (or 404s on) whatever name the body carries. No cross-check between the
     * two.
     * @param req the inbound request; body is the replacement definition, Content-Type picks
     * the decoder, Accept picks the reply format.
     * @param res the response — 200 with the updated definition, 400 on a parse failure, 422 on
     * a validation failure, or 404 if the connector's update() can't find that name.
     */
    // Not noexcept — same reasoning as create_definition() above.
    void update_definition(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        // decode the replacement body — 400 if it doesn't parse as a TaskDef
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::TaskDef>(content_type, body);
        if (!parsed) {
            core::logger::warning("engine", "task/update bad request: {}", parsed.error());
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            return;
        }

        // same validation pass as create — 422 if it fails
        if (auto validate = parsed->validate(); !validate) {
            core::logger::warning("engine", "task/update invalid: {}", validate.error());
            reply(res, serde::Ser::serialize_error(accept, validate.error()),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            return;
        }

        // key off the name in the body, not the URL segment — update() 404s if it's not there.
        // Callback logs and serializes the reply (may throw); not noexcept, same reasoning as above.
        m_ctx.get().get_connector().update<model::TaskDef>(*parsed, [&](bool oke) {
            if (!oke) {
                core::logger::warning("engine", "task/update not found: '{}'", parsed->get_name());
                reply(res, serde::Ser::serialize_error(accept, "not found"),
                      interfaces::io::types::Status::NOT_FOUND);
                return;
            }
            reply(res, serde::Ser::serialize(accept, *parsed));
        });
    }

    /**
     * @brief Handles `DELETE /api/v1/tasks/:name` — removes a TaskDef by name.
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

        // delete by name — 404 if there was nothing to delete, else 204 and a log line.
        // Callback logs and serializes the error reply (may throw); not noexcept, same
        // reasoning as the callbacks above.
        m_ctx.get().get_connector().remove<model::TaskDef>(name, [&](bool oke) {
            if (!oke) {
                core::logger::warning("engine", "task/remove not found: '{}'", name);
                reply(res, serde::Ser::serialize_error(accept, "not found"),
                      interfaces::io::types::Status::NOT_FOUND);
                return;
            }
            core::logger::info("engine", "task deleted: '{}'", name);
            res.set_status(interfaces::io::types::Status::NO_CONTENT);
        });
    }

    /**
     * @brief Handles `GET /api/v1/tasks/queue/:type` — claims one SCHEDULED TaskInstance whose
     * definition's worker_type matches `:type`, flips it to IN_PROGRESS, and hands it back.
     * @note The predicate/sorter passed here only actually run in Connector::find_first()'s
     * no-database (local-store) branch — when a real db is configured, the SQL built from
     * `options` (the JOIN + WHERE clause above) does the filtering instead, and this predicate
     * never fires. Keep both matching logics in sync by hand, that's on whoever touches this.
     * @param req the inbound request; path supplies the worker type, Accept header picks the
     * format.
     * @param res the response — 200 with the claimed instance, 204 if nothing's queued for that
     * worker type, or 500 if the claim's update() fails after a match was already found.
     */
    // Not noexcept — body builds a std::format'd query filter string, serializes replies, and
    // calls into nested connector callbacks, any of which may throw. Same reasoning as
    // create_definition() above: HandlerFn doesn't require a noexcept target.
    void poll(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto worker_type = std::string{target.substr(target.rfind('/') + 1)};

        // build the db-side filter — only SCHEDULED instances whose def's worker_type matches, oldest seq first
        auto options =
            serde::QueryOptions{}
                .add_join(
                    "JOIN task_definitions ON task_instances.def_name = task_definitions.name")
                .add_where(std::format(
                    "task_instances.status = 'SCHEDULED' AND task_definitions.worker_type = '{}'",
                    worker_type))
                .add_order_by("task_instances.seq");

        m_ctx.get().get_connector().find_first<model::TaskInstance>(
            std::move(options),
            // local-store fallback predicate — hand-mirrors the SQL filter above for the no-db path
            [this, worker_type](const model::TaskInstance &instance) noexcept {
                if (instance.get_status() != model::TaskStatus::SCHEDULED) {
                    return false;
                }
                bool worker_matches = false;
                m_ctx.get().get_connector().find<model::TaskDef>(
                    instance.get_def_name(),
                    [&worker_type,
                     &worker_matches](std::optional<model::TaskDef> definition) noexcept {
                        worker_matches = definition && definition->get_worker_type() == worker_type;
                    });
                return worker_matches;
            },
            // local-store fallback sorter — same seq ordering as add_order_by() above
            [](const model::TaskInstance &lhs, const model::TaskInstance &rhs) noexcept {
                return lhs.get_seq() < rhs.get_seq();
            },
            [&, accept](std::optional<model::TaskInstance> found) mutable noexcept {
                // nothing queued for this worker type — 204, nothing to claim
                if (!found) {
                    res.set_status(interfaces::io::types::Status::NO_CONTENT);
                    return;
                }
                // flip it to IN_PROGRESS and persist the claim before handing it back
                found->set_status(model::TaskStatus::IN_PROGRESS);
                auto claimed = std::move(*found);
                // Callback serializes the reply (may throw); not noexcept, same reasoning as
                // the other connector callbacks in this file.
                m_ctx.get().get_connector().update<model::TaskInstance>(
                    claimed, [&res, accept, claimed](bool oke) mutable {
                        if (!oke) {
                            reply(res, serde::Ser::serialize_error(accept, "claim failed"),
                                  interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
                            return;
                        }
                        reply(res, serde::Ser::serialize(accept, claimed));
                    });
            });
    }

    // POST /api/v1/tasks/:name/enqueue — create a new TaskInstance in SCHEDULED status
    /**
     * @brief Verifies the named TaskDef exists, then inserts a new SCHEDULED TaskInstance for
     * it, with optional input_data/seq pulled from an optional JSON body.
     * @warning `def_name` is carved out by hand via two `rfind('/')` calls instead of real
     * route-param binding — brittle if this route's path shape ever changes out from under the
     * string math. Same pattern shows up again in submit_result() below and in
     * WorkflowHandler::start_execution().
     * @warning A non-empty but malformed body just gets swallowed — if deserialize() fails the
     * `if (auto parsed = ...)` simply doesn't fire, and input_data/seq quietly fall back to
     * their defaults instead of the request getting a 400. Same body-swallowing move as
     * WorkflowHandler::start_execution(). Client sends garbage, task still gets enqueued like
     * nothing happened.
     * @param req the inbound request; path supplies the definition name, an optional JSON body
     * supplies input_data/seq, Accept picks the reply format.
     * @param res the response — 201 with the new instance, 404 if the named definition doesn't
     * exist, or 500 if the insert fails.
     */
    // Not noexcept — body does string/JSON parsing and logging, any of which may throw. Same
    // reasoning as create_definition() above: HandlerFn doesn't require a noexcept target.
    void enqueue_task(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto target = req.get_path();
        // Path is /api/v1/tasks/:name/enqueue — def_name is second-to-last segment
        auto last_slash = target.rfind('/');
        auto prev_slash = target.rfind('/', last_slash > 0 ? last_slash - 1 : 0);
        auto def_name = std::string{target.substr(prev_slash + 1, last_slash - prev_slash - 1)};

        // Verify the task definition exists
        bool def_exists = false;
        m_ctx.get().get_connector().find<model::TaskDef>(
            def_name, [&](const std::optional<model::TaskDef> &result) noexcept {
                def_exists = result.has_value();
            });

        // no such definition — 404 before we ever try to enqueue anything against it
        if (!def_exists) {
            reply(res, serde::Ser::serialize_error(accept, "task definition not found"),
                  interfaces::io::types::Status::NOT_FOUND);
            return;
        }

        // Parse optional body — input_data and seq
        std::unordered_map<std::string, std::string> input_data;
        std::uint32_t seq = 0;

        auto body = flatten_body(req);
        if (!body.empty()) {
            if (auto parsed = serde::Ser::deserialize<TaskEnqueueBody>(content_type, body)) {
                input_data = parsed->get_input_data();
                seq = parsed->get_seq();
            }
        }

        // stamp a fresh SCHEDULED instance with whatever input/seq we picked up (or the defaults)
        model::TaskInstance inst;
        inst.set_task_id(model::generate_id());
        inst.set_def_name(def_name);
        inst.set_status(model::TaskStatus::SCHEDULED);
        inst.set_seq(seq);
        inst.set_input_data(std::move(input_data));

        // persist it and reply with what got created — same insert/reply motion as create_definition().
        // Callback logs and serializes the reply (may throw); not noexcept, same reasoning as above.
        m_ctx.get().get_connector().insert<model::TaskInstance>(inst, [&](bool oke) {
            if (!oke) {
                core::logger::error("engine", "task/enqueue insert failed for '{}'", def_name);
                reply(res, serde::Ser::serialize_error(accept, "insert failed"),
                      interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
                return;
            }
            core::logger::info("engine", "task enqueued: '{}' id={}", def_name, inst.get_task_id());
            reply(res, serde::Ser::serialize(accept, inst), interfaces::io::types::Status::CREATED);
        });
    }

    /**
     * @brief Handles `POST /api/v1/tasks/:id/result` — records a worker's reported result for a
     * TaskInstance, mapping the TaskResult onto its matching terminal TaskStatus before
     * updating the row.
     * @warning Same rfind-twice path slicing as enqueue_task() to pull `task_id` out — no
     * dedicated param extraction here either.
     * @param req the inbound request; path supplies the task instance id, body is the submitted
     * result, Content-Type picks the decoder, Accept picks the reply format.
     * @param res the response — 200 with the updated instance, 400 on a parse failure, or 404 if
     * the instance id doesn't exist (on the initial lookup or on the follow-up update()).
     */
    // Not noexcept — body does string/JSON parsing and calls into nested connector callbacks
    // that log/serialize, any of which may throw. Same reasoning as create_definition() above.
    void submit_result(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        // same second-to-last-segment slicing as enqueue_task() to pull task_id out
        auto target = req.get_path();
        auto last = target.rfind('/');
        auto before = target.rfind('/', last > 0 ? last - 1 : 0);
        auto task_id = std::string{target.substr(before + 1, last - before - 1)};

        // decode the submitted result — 400 if it doesn't parse
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<TaskSubmitBody>(content_type, body);
        if (!parsed) {
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            return;
        }

        // Callback formats/serializes the reply and calls into a further nested connector
        // callback, any of which may throw; not noexcept, same reasoning as above.
        m_ctx.get().get_connector().find<model::TaskInstance>(
            task_id, [&, accept, submit = std::move(*parsed)](
                         std::optional<model::TaskInstance> found) mutable {
                // no instance with that id — 404, nothing to record a result against
                if (!found) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    return;
                }

                // maps the worker's reported result onto the matching terminal status
                constexpr auto TO_STATUS =
                    [](model::TaskResult result) noexcept -> model::TaskStatus {
                    switch (result) {
                    case model::TaskResult::SUCCESS:
                        return model::TaskStatus::COMPLETED;
                    case model::TaskResult::FAILURE:
                        return model::TaskStatus::FAILED;
                    case model::TaskResult::TIMEOUT:
                        return model::TaskStatus::TIMED_OUT;
                    case model::TaskResult::SKIPPED:
                        return model::TaskStatus::SKIPPED;
                    }
                    return model::TaskStatus::FAILED;
                };

                // stamp the terminal status + output data, then persist it
                found->set_status(TO_STATUS(submit.get_result()));
                found->set_output_data(submit.get_output_data());
                auto updated = std::move(*found);

                // Callback serializes the reply, which may throw; not noexcept, same reasoning
                // as the other connector callbacks in this file.
                m_ctx.get().get_connector().update<model::TaskInstance>(
                    updated, [&res, accept, updated](bool oke) mutable {
                        if (!oke) {
                            reply(res, serde::Ser::serialize_error(accept, "not found"),
                                  interfaces::io::types::Status::NOT_FOUND);
                            return;
                        }
                        reply(res, serde::Ser::serialize(accept, updated));
                    });
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
     * `static_cast<char>` per byte — this is what every handler above hands off to
     * serde::Ser::deserialize(), which wants a `string_view` over already-materialized text.
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

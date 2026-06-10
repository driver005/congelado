module engine.handler.task;

@nogc nothrow:

import interfaces.interfaces;
import model.model;
import serde.serde;
import core.logger.logger;
import engine.context;

// ─── TaskSubmitBody ──────────────────────────────────────────────────────────────

// PORT-NOTE: serde::Serializable<TaskSubmitBody> specialization preserved as comment;
// D does not have C++ template specialization — wire via Ser when serde is ported.
class TaskSubmitBody {
  public:
    void set_result(TaskResult result) { m_result = result; }
    void set_output_data(const(char)[][const(char)[]] data) {
        m_output_data = data;
    }
    TaskResult get_result() const { return m_result; }
    ref const(const(char)[][const(char)[]]) get_output_data() const { return m_output_data; }

  private:
    TaskResult m_result = TaskResult.SUCCESS;
    const(char)[][const(char)[]] m_output_data;
}

// ─── TaskHandler ──────────────────────────────────────────────────────────────

// Routes:
//   GET    /api/v1/tasks/:name         → get_definition
//   POST   /api/v1/tasks               → create_definition
//   PUT    /api/v1/tasks/:name         → update_definition
//   DELETE /api/v1/tasks/:name         → remove_definition
//   GET    /api/v1/tasks/queue/:type   → poll
//   POST   /api/v1/tasks/:id/result    → submit_result
class TaskHandler(Protocol) {
  public:
    this(ref EngineContext ctx) { m_ctx = &ctx; }

    void get_definition(ref IRequest!Protocol req,
                        ref IResponse!Protocol res) {
        auto accept = req.find_header("accept");
        auto target = req.get_target();
        size_t slash = last_slash(target);
        auto name = target[slash + 1 .. $];

        m_ctx.get_connector().find!TaskDef(
            name, (import("util.optional") Optional!TaskDef result) {
                if (!result.has_value()) {
                    reply(res, Ser.serialize_error(accept, "not found"),
                          Status.NOT_FOUND);
                    return;
                }
                reply(res, Ser.serialize(accept, result.value()));
            });
    }

    void create_definition(ref IRequest!Protocol req,
                           ref IResponse!Protocol res) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        auto body = flatten_body(req);
        auto parsed = Ser.deserialize!TaskDef(content_type, body);
        if (!parsed.ok()) {
            warning("engine", "task/create bad request");
            reply(res, Ser.serialize_error(accept, parsed.error()),
                  Status.BAD_REQUEST);
            return;
        }

        // PORT-NOTE: parsed->validate() preserved; TaskDef.validate() returns Result
        auto validate = parsed.value().validate();
        if (!validate.ok()) {
            warning("engine", "task/create invalid");
            reply(res, Ser.serialize_error(accept, validate.error()),
                  Status.UNPROCESSABLE_CONTENT);
            return;
        }

        m_ctx.get_connector().insert!TaskDef(parsed.value(), (bool okee) {
            if (!okee) {
                error("engine", "task/create db insert failed");
                reply(res, Ser.serialize_error(accept, "insert failed"),
                      Status.INTERNAL_SERVER_ERROR);
                return;
            }
            info("engine", "task created");
            reply(res, Ser.serialize(accept, parsed.value()), Status.CREATED);
        });
    }

    void update_definition(ref IRequest!Protocol req,
                           ref IResponse!Protocol res) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        auto body = flatten_body(req);
        auto parsed = Ser.deserialize!TaskDef(content_type, body);
        if (!parsed.ok()) {
            warning("engine", "task/update bad request");
            reply(res, Ser.serialize_error(accept, parsed.error()),
                  Status.BAD_REQUEST);
            return;
        }

        auto validate = parsed.value().validate();
        if (!validate.ok()) {
            warning("engine", "task/update invalid");
            reply(res, Ser.serialize_error(accept, validate.error()),
                  Status.UNPROCESSABLE_CONTENT);
            return;
        }

        m_ctx.get_connector().update!TaskDef(parsed.value(), (bool okee) {
            if (!okee) {
                warning("engine", "task/update not found");
                reply(res, Ser.serialize_error(accept, "not found"),
                      Status.NOT_FOUND);
                return;
            }
            reply(res, Ser.serialize(accept, parsed.value()));
        });
    }

    void remove_definition(ref IRequest!Protocol req,
                           ref IResponse!Protocol res) {
        auto accept = req.find_header("accept");
        auto target = req.get_target();
        auto name = target[last_slash(target) + 1 .. $];

        m_ctx.get_connector().remove!TaskDef(name, (bool okee) {
            if (!okee) {
                warning("engine", "task/remove not found");
                reply(res, Ser.serialize_error(accept, "not found"),
                      Status.NOT_FOUND);
                return;
            }
            info("engine", "task deleted");
            res.set_status(Status.NO_CONTENT);
        });
    }

    void poll(ref IRequest!Protocol req, ref IResponse!Protocol res) {
        auto accept = req.find_header("accept");
        auto target = req.get_target();
        auto worker_type = target[last_slash(target) + 1 .. $];

        // PORT-NOTE: QueryOptions + find_first translated structurally;
        // The predicate/comparator/callback pattern mirrors C++ lambda captures.
        auto options = QueryOptions()
            .add_join("JOIN task_definitions ON task_instances.def_name = task_definitions.name")
            .add_where("task_instances.status = 'SCHEDULED' AND task_definitions.worker_type = '?'",
                       worker_type)
            .add_order_by("task_instances.seq");

        m_ctx.get_connector().find_first!TaskInstance(
            options,
            (ref const(TaskInstance) instance) {
                if (instance.get_status() != TaskStatus.SCHEDULED) {
                    return false;
                }
                bool worker_matches = false;
                m_ctx.get_connector().find!TaskDef(
                    instance.get_def_name(),
                    (import("util.optional") Optional!TaskDef definition) {
                        worker_matches = definition.has_value() &&
                            definition.value().get_worker_type() == worker_type;
                    });
                return worker_matches;
            },
            (ref const(TaskInstance) lhs, ref const(TaskInstance) rhs) {
                return lhs.get_seq() < rhs.get_seq();
            },
            (import("util.optional") Optional!TaskInstance found) {
                if (!found.has_value()) {
                    res.set_status(Status.NO_CONTENT);
                    return;
                }
                found.value().set_status(TaskStatus.IN_PROGRESS);
                auto claimed = found.value();
                m_ctx.get_connector().update!TaskInstance(
                    claimed, (bool oke) {
                        if (!oke) {
                            reply(res, Ser.serialize_error(accept, "claim failed"),
                                  Status.INTERNAL_SERVER_ERROR);
                            return;
                        }
                        reply(res, Ser.serialize(accept, claimed));
                    });
            });
    }

    void submit_result(ref IRequest!Protocol req,
                       ref IResponse!Protocol res) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto target = req.get_target();
        auto last = last_slash(target);
        auto before = last_slash_before(target, last);
        auto task_id = target[before + 1 .. last];

        auto body = flatten_body(req);
        auto parsed = Ser.deserialize!TaskSubmitBody(content_type, body);
        if (!parsed.ok()) {
            reply(res, Ser.serialize_error(accept, parsed.error()),
                  Status.BAD_REQUEST);
            return;
        }

        auto submit = parsed.value();
        m_ctx.get_connector().find!TaskInstance(
            task_id, (import("util.optional") Optional!TaskInstance found) {
                if (!found.has_value()) {
                    reply(res, Ser.serialize_error(accept, "not found"),
                          Status.NOT_FOUND);
                    return;
                }

                static TaskStatus to_status(TaskResult result) {
                    final switch (result) {
                    case TaskResult.SUCCESS: return TaskStatus.COMPLETED;
                    case TaskResult.FAILURE: return TaskStatus.FAILED;
                    case TaskResult.TIMEOUT: return TaskStatus.TIMED_OUT;
                    case TaskResult.SKIPPED: return TaskStatus.SKIPPED;
                    }
                }

                found.value().set_status(to_status(submit.get_result()));
                found.value().set_output_data(submit.get_output_data());
                auto updated = found.value();

                m_ctx.get_connector().update!TaskInstance(
                    updated, (bool oke) {
                        if (!oke) {
                            reply(res, Ser.serialize_error(accept, "not found"),
                                  Status.NOT_FOUND);
                            return;
                        }
                        reply(res, Ser.serialize(accept, updated));
                    });
            });
    }

  private:
    // PORT-NOTE: C++ used std::reference_wrapper<EngineContext>; D uses raw pointer.
    EngineContext* m_ctx;

    static void reply(ref IResponse!Protocol res, ubyte[] bytes,
                      Status status = Status.OK) {
        res.set_body(bytes);
        res.set_status(status);
    }

    static ubyte[] flatten_body(ref IRequest!Protocol req) {
        auto view = req.get_body();
        // PORT-NOTE: C++ accumulated body bytes into std::vector<uint8_t>;
        // D uses fixed-size stack buffer[4096] to avoid GC.
        static ubyte[4096] buf;
        size_t buf_len = 0;
        foreach (b; view) {
            assert(buf_len < buf.length, "flatten_body overflow");
            buf[buf_len++] = b;
        }
        return buf[0 .. buf_len];
    }

    // Returns index of last '/' in slice, or 0 if none.
    static size_t last_slash(const(char)[] s) {
        for (ptrdiff_t i = cast(ptrdiff_t) s.length - 1; i >= 0; --i) {
            if (s[i] == '/') return cast(size_t) i;
        }
        return 0;
    }

    // Returns index of last '/' strictly before position `before`, or 0 if none.
    static size_t last_slash_before(const(char)[] s, size_t before) {
        if (before == 0) return 0;
        for (ptrdiff_t i = cast(ptrdiff_t)(before) - 1; i >= 0; --i) {
            if (s[i] == '/') return cast(size_t) i;
        }
        return 0;
    }
}

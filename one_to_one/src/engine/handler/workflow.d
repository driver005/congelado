module engine.handler.workflow;

@nogc nothrow:

import interfaces.interfaces;
import model.model;
import serde.serde;
import core.logger.logger;
import engine.context;

// ─── WorkflowStartBody ────────────────────────────────────────────────────────

// PORT-NOTE: serde::Serializable<WorkflowStartBody> specialization preserved as comment;
// D does not have C++ template specialization — wire via Ser when serde is ported.
class WorkflowStartBody {
  public:
    void set_variables(const(char)[][const(char)[]] value) {
        m_variables = value;
    }
    ref const(const(char)[][const(char)[]]) get_variables() const { return m_variables; }

  private:
    const(char)[][const(char)[]] m_variables;
}

// ─── WorkflowHandler ─────────────────────────────────────────────────────────

// Routes:
//   GET    /api/v1/workflows/:name          → get_definition
//   POST   /api/v1/workflows                → create_definition
//   PUT    /api/v1/workflows/:name          → update_definition
//   DELETE /api/v1/workflows/:name          → remove_definition
//   POST   /api/v1/workflows/:name/start    → start_execution
//   GET    /api/v1/workflows/exec/:id       → get_execution
//   DELETE /api/v1/workflows/exec/:id       → terminate_execution
class WorkflowHandler(Protocol) {
  public:
    this(ref EngineContext ctx) { m_ctx = &ctx; }

    void get_definition(ref IRequest!Protocol req,
                        ref IResponse!Protocol res) {
        auto accept = req.find_header("accept");
        auto target = req.get_target();
        auto name = target[last_slash(target) + 1 .. $];

        m_ctx.get_connector().find!WorkflowDef(
            name, (import("util.optional") Optional!WorkflowDef result) {
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
        auto parsed = Ser.deserialize!WorkflowDef(content_type, body);
        if (!parsed.ok()) {
            warning("engine", "wf/create bad request");
            reply(res, Ser.serialize_error(accept, parsed.error()),
                  Status.BAD_REQUEST);
            return;
        }

        auto value = parsed.value().validate();
        if (!value.ok()) {
            warning("engine", "wf/create invalid");
            reply(res, Ser.serialize_error(accept, value.error()),
                  Status.UNPROCESSABLE_CONTENT);
            return;
        }

        m_ctx.get_connector().insert!WorkflowDef(parsed.value(), (bool oke) {
            if (!oke) {
                error("engine", "wf/create db insert failed");
                reply(res, Ser.serialize_error(accept, "insert failed"),
                      Status.INTERNAL_SERVER_ERROR);
                return;
            }
            info("engine", "workflow created");
            reply(res, Ser.serialize(accept, parsed.value()), Status.CREATED);
        });
    }

    void update_definition(ref IRequest!Protocol req,
                           ref IResponse!Protocol res) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        auto body = flatten_body(req);
        auto parsed = Ser.deserialize!WorkflowDef(content_type, body);
        if (!parsed.ok()) {
            reply(res, Ser.serialize_error(accept, parsed.error()),
                  Status.BAD_REQUEST);
            return;
        }

        auto value = parsed.value().validate();
        if (!value.ok()) {
            reply(res, Ser.serialize_error(accept, value.error()),
                  Status.UNPROCESSABLE_CONTENT);
            return;
        }

        m_ctx.get_connector().update!WorkflowDef(parsed.value(), (bool oke) {
            if (!oke) {
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

        m_ctx.get_connector().remove!WorkflowDef(name, (bool oke) {
            if (!oke) {
                reply(res, Ser.serialize_error(accept, "not found"),
                      Status.NOT_FOUND);
                return;
            }
            res.set_status(Status.NO_CONTENT);
        });
    }

    // Path: /api/v1/workflows/:name/start — def_name is the segment before "start".
    void start_execution(ref IRequest!Protocol req,
                         ref IResponse!Protocol res) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto target = req.get_target();
        auto last = last_slash(target);
        auto before = last_slash_before(target, last);
        auto def_name = target[before + 1 .. last];

        const(char)[][const(char)[]] variables;
        auto body = flatten_body(req);
        if (body.length > 0) {
            auto parsed = Ser.deserialize!WorkflowStartBody(content_type, body);
            if (parsed.ok()) {
                variables = parsed.value().get_variables();
            }
        }

        // PORT-NOTE: WorkflowExecution is a class; heap-allocated via make!
        import util.alloc : make;
        auto exec = make!WorkflowExecution();
        exec.set_exec_id(generate_id());
        exec.set_def_name(def_name);
        exec.set_status(WorkflowStatus.RUNNING);
        exec.set_variables(variables);

        ExecutionTimings timings;
        timings.set_started_at(current_time_ms());
        exec.set_timings(timings);

        m_ctx.get_connector().insert!WorkflowExecution(*exec, (bool oke) {
            if (!oke) {
                error("engine", "wf/start insert failed");
                reply(res, Ser.serialize_error(accept, "insert failed"),
                      Status.INTERNAL_SERVER_ERROR);
                return;
            }
            info("engine", "exec started");
            reply(res, Ser.serialize(accept, *exec), Status.ACCEPTED);
        });
    }

    // Path: /api/v1/workflows/exec/:id — exec_id is the last segment.
    void get_execution(ref IRequest!Protocol req,
                       ref IResponse!Protocol res) {
        auto accept = req.find_header("accept");
        auto target = req.get_target();
        auto exec_id_str = target[last_slash(target) + 1 .. $];

        m_ctx.get_connector().find!WorkflowExecution(
            exec_id_str, (import("util.optional") Optional!WorkflowExecution result) {
                if (!result.has_value()) {
                    reply(res, Ser.serialize_error(accept, "not found"),
                          Status.NOT_FOUND);
                    return;
                }
                reply(res, Ser.serialize(accept, result.value()));
            });
    }

    // Path: /api/v1/workflows/exec/:id — exec_id is the last segment.
    void terminate_execution(ref IRequest!Protocol req,
                             ref IResponse!Protocol res) {
        auto accept = req.find_header("accept");
        auto target = req.get_target();
        auto exec_id_str = target[last_slash(target) + 1 .. $];

        import util.optional : Optional, none;
        Optional!WorkflowExecution found = none!WorkflowExecution();
        bool handled = false;
        m_ctx.get_connector().find!WorkflowExecution(
            exec_id_str, (Optional!WorkflowExecution result) {
                if (!result.has_value()) {
                    warning("engine", "wf/terminate not found");
                    reply(res, Ser.serialize_error(accept, "not found"),
                          Status.NOT_FOUND);
                    handled = true;
                    return;
                }
                if (is_terminal(result.value().get_status())) {
                    warning("engine", "wf/terminate already terminal");
                    reply(res, Ser.serialize_error(accept, "already in terminal state"),
                          Status.CONFLICT);
                    handled = true;
                    return;
                }
                found = result;
            });

        if (handled) {
            return;
        }

        found.value().set_status(WorkflowStatus.TERMINATED);
        m_ctx.get_connector().update!WorkflowExecution(
            found.value(), (bool oke) {
                if (!oke) {
                    reply(res, Ser.serialize_error(accept, "not found"),
                          Status.NOT_FOUND);
                    return;
                }
                info("engine", "exec terminated");
                reply(res, Ser.serialize(accept, found.value()));
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
        // PORT-NOTE: @nogc append deferred — wire with util.alloc in Run 2
        ubyte[] out_;
        foreach (b; view) {
            out_ ~= b;
        }
        return out_;
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

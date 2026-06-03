export module engine:workflow;

import std;
import interfaces;
import model;
import shared;
import serde;
import :context;

// ─── WorkflowStartBody ────────────────────────────────────────────────────────

namespace engine {

class WorkflowStartBody {
  public:
    void set_variables(std::unordered_map<std::string, std::string> v) noexcept {
        m_variables = std::move(v);
    }
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
template <typename Protocol>
class WorkflowHandler {
  public:
    explicit WorkflowHandler(EngineContext &ctx) noexcept : m_ctx(ctx) {}

    void get_definition(interfaces::IRequest<Protocol> &req,
                        interfaces::IResponse<Protocol> &res) noexcept {
        auto accept = req.find_header("accept");
        auto target = req.get_target();
        auto name = std::string{target.substr(target.rfind('/') + 1)};

        if (!m_ctx.get().get_db()) {
            reply(res, serde::Ser::serialize_error(accept, "service unavailable"),
                  interfaces::Status::SERVICE_UNAVAILABLE);
            return;
        }

        m_ctx.get().get_workflow_def_connector().find(
            name, [&](std::optional<model::WorkflowDef> result) noexcept {
                if (!result) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::Status::NOT_FOUND);
                    return;
                }
                reply(res, serde::Ser::serialize(accept, *result));
            });
        m_ctx.get().get_workflow_def_connector().flush();
    }

    void create_definition(interfaces::IRequest<Protocol> &req,
                           interfaces::IResponse<Protocol> &res) noexcept {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        if (!m_ctx.get().get_db()) {
            reply(res, serde::Ser::serialize_error(accept, "service unavailable"),
                  interfaces::Status::SERVICE_UNAVAILABLE);
            return;
        }

        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::WorkflowDef>(content_type, body);
        if (!parsed) {
            reply(res, serde::Ser::serialize_error(accept, "invalid body"),
                  interfaces::Status::BAD_REQUEST);
            return;
        }

        if (auto v = parsed->validate(); !v) {
            reply(res, serde::Ser::serialize_error(accept, v.error()),
                  interfaces::Status::UNPROCESSABLE_CONTENT);
            return;
        }

        m_ctx.get().get_workflow_def_connector().insert(*parsed, [&](bool ok) noexcept {
            if (!ok) {
                reply(res, serde::Ser::serialize_error(accept, "insert failed"),
                      interfaces::Status::INTERNAL_SERVER_ERROR);
                return;
            }
            reply(res, serde::Ser::serialize(accept, *parsed), interfaces::Status::CREATED);
        });
        m_ctx.get().get_workflow_def_connector().flush();
    }

    void update_definition(interfaces::IRequest<Protocol> &req,
                           interfaces::IResponse<Protocol> &res) noexcept {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        if (!m_ctx.get().get_db()) {
            reply(res, serde::Ser::serialize_error(accept, "service unavailable"),
                  interfaces::Status::SERVICE_UNAVAILABLE);
            return;
        }

        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::WorkflowDef>(content_type, body);
        if (!parsed) {
            reply(res, serde::Ser::serialize_error(accept, "invalid body"),
                  interfaces::Status::BAD_REQUEST);
            return;
        }

        if (auto v = parsed->validate(); !v) {
            reply(res, serde::Ser::serialize_error(accept, v.error()),
                  interfaces::Status::UNPROCESSABLE_CONTENT);
            return;
        }

        m_ctx.get().get_workflow_def_connector().update(*parsed, [&](bool ok) noexcept {
            if (!ok) {
                reply(res, serde::Ser::serialize_error(accept, "not found"),
                      interfaces::Status::NOT_FOUND);
                return;
            }
            reply(res, serde::Ser::serialize(accept, *parsed));
        });
        m_ctx.get().get_workflow_def_connector().flush();
    }

    void remove_definition(interfaces::IRequest<Protocol> &req,
                           interfaces::IResponse<Protocol> &res) noexcept {
        auto accept = req.find_header("accept");
        auto target = req.get_target();
        auto name = std::string{target.substr(target.rfind('/') + 1)};

        if (!m_ctx.get().get_db()) {
            reply(res, serde::Ser::serialize_error(accept, "service unavailable"),
                  interfaces::Status::SERVICE_UNAVAILABLE);
            return;
        }

        m_ctx.get().get_workflow_def_connector().remove(name, [&](bool ok) noexcept {
            if (!ok) {
                reply(res, serde::Ser::serialize_error(accept, "not found"),
                      interfaces::Status::NOT_FOUND);
                return;
            }
            res.set_status(interfaces::Status::NO_CONTENT);
        });
        m_ctx.get().get_workflow_def_connector().flush();
    }

    // Path: /api/v1/workflows/:name/start — def_name is the segment before "start".
    void start_execution(interfaces::IRequest<Protocol> &req,
                         interfaces::IResponse<Protocol> &res) noexcept {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto target = req.get_target();
        auto last = target.rfind('/');
        auto before = target.rfind('/', last > 0 ? last - 1 : 0);
        auto def_name = std::string{target.substr(before + 1, last - before - 1)};

        if (!m_ctx.get().get_db()) {
            reply(res, serde::Ser::serialize_error(accept, "service unavailable"),
                  interfaces::Status::SERVICE_UNAVAILABLE);
            return;
        }

        std::unordered_map<std::string, std::string> variables;
        auto body = flatten_body(req);
        if (!body.empty()) {
            if (auto parsed = serde::Ser::deserialize<WorkflowStartBody>(content_type, body))
                variables = parsed->get_variables();
        }

        model::WorkflowExecution exec;
        exec.set_exec_id(model::generate_id());
        exec.set_def_name(def_name);
        exec.set_status(model::WorkflowStatus::RUNNING);
        exec.set_variables(std::move(variables));

        model::ExecutionTimings timings;
        timings.set_started_at(std::chrono::system_clock::now());
        exec.set_timings(timings);

        m_ctx.get().get_exec_connector().insert(exec, [&](bool ok) noexcept {
            if (!ok) {
                reply(res, serde::Ser::serialize_error(accept, "insert failed"),
                      interfaces::Status::INTERNAL_SERVER_ERROR);
                return;
            }
            reply(res, serde::Ser::serialize(accept, exec), interfaces::Status::ACCEPTED);
        });
        m_ctx.get().get_exec_connector().flush();
    }

    // Path: /api/v1/workflows/exec/:id — exec_id is the last segment.
    void get_execution(interfaces::IRequest<Protocol> &req,
                       interfaces::IResponse<Protocol> &res) noexcept {
        auto accept = req.find_header("accept");
        auto target = req.get_target();
        auto exec_id_str = std::string{target.substr(target.rfind('/') + 1)};

        if (!m_ctx.get().get_db()) {
            reply(res, serde::Ser::serialize_error(accept, "service unavailable"),
                  interfaces::Status::SERVICE_UNAVAILABLE);
            return;
        }

        m_ctx.get().get_exec_connector().find(
            exec_id_str, [&](std::optional<model::WorkflowExecution> result) noexcept {
                if (!result) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::Status::NOT_FOUND);
                    return;
                }
                reply(res, serde::Ser::serialize(accept, *result));
            });
        m_ctx.get().get_exec_connector().flush();
    }

    // Path: /api/v1/workflows/exec/:id — exec_id is the last segment.
    void terminate_execution(interfaces::IRequest<Protocol> &req,
                             interfaces::IResponse<Protocol> &res) noexcept {
        auto accept = req.find_header("accept");
        auto target = req.get_target();
        auto exec_id_str = std::string{target.substr(target.rfind('/') + 1)};

        if (!m_ctx.get().get_db()) {
            reply(res, serde::Ser::serialize_error(accept, "service unavailable"),
                  interfaces::Status::SERVICE_UNAVAILABLE);
            return;
        }

        // Guard: reject if not found or already in a terminal state.
        bool handled = false;
        m_ctx.get().get_exec_connector().find(
            exec_id_str, [&](std::optional<model::WorkflowExecution> result) noexcept {
                if (!result) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::Status::NOT_FOUND);
                    handled = true;
                    return;
                }
                if (model::is_terminal(result->get_status())) {
                    reply(res, serde::Ser::serialize_error(accept, "already in terminal state"),
                          interfaces::Status::CONFLICT);
                    handled = true;
                }
            });
        m_ctx.get().get_exec_connector().flush();

        if (handled)
            return;

        auto cache_key = serde::Cache::cache_key<model::WorkflowExecution>(exec_id_str);
        auto sql = std::format("UPDATE workflow_executions SET status = 'TERMINATED' "
                               "WHERE exec_id = '{}' RETURNING row_to_json(workflow_executions.*)",
                               exec_id_str);

        m_ctx.get().get_db()->query(sql, [&](std::string_view result) noexcept {
            if (result.empty()) {
                reply(res, serde::Ser::serialize_error(accept, "not found"),
                      interfaces::Status::NOT_FOUND);
                return;
            }
            auto v = serde::Json::decode<model::WorkflowExecution>(result);
            if (!v) {
                reply(res, serde::Ser::serialize_error(accept, "decode error"),
                      interfaces::Status::INTERNAL_SERVER_ERROR);
                return;
            }
            if (m_ctx.get().get_cache())
                m_ctx.get().get_cache()->remove(cache_key, [](std::string_view) noexcept {});
            reply(res, serde::Ser::serialize(accept, *v));
        });
    }

  private:
    std::reference_wrapper<EngineContext> m_ctx;

    static void reply(interfaces::IResponse<Protocol> &res, std::vector<std::byte> bytes,
                      interfaces::Status status = interfaces::Status::OK) noexcept {
        res.set_body(std::move(bytes));
        res.set_status(status);
    }

    static std::string flatten_body(interfaces::IRequest<Protocol> &req) noexcept {
        std::string out;
        auto &view = req.get_body();
        out.reserve(view.size());
        for (std::byte b : view)
            out += static_cast<char>(b);
        return out;
    }
};

} // namespace engine

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
    void set_variables(std::unordered_map<std::string, std::string> value) noexcept {
        m_variables = std::move(value);
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
class WorkflowHandler {
  public:
    explicit WorkflowHandler(EngineContext &ctx) noexcept : m_ctx(ctx) {}

    void get_definition(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto name = std::string{target.substr(target.rfind('/') + 1)};

        m_ctx.get().get_connector().find<model::WorkflowDef>(
            name, [&](std::optional<model::WorkflowDef> result) noexcept {
                if (!result) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    return;
                }
                reply(res, serde::Ser::serialize(accept, *result));
            });
    }

    void create_definition(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::WorkflowDef>(content_type, body);
        if (!parsed) {
            core::logger::warning("engine", "wf/create bad request: {}", parsed.error());
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            return;
        }

        if (auto value = parsed->validate(); !value) {
            core::logger::warning("engine", "wf/create invalid: {}", value.error());
            reply(res, serde::Ser::serialize_error(accept, value.error()),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            return;
        }

        m_ctx.get().get_connector().insert<model::WorkflowDef>(*parsed, [&](bool oke) noexcept {
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

    void update_definition(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::WorkflowDef>(content_type, body);
        if (!parsed) {
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            return;
        }

        if (auto value = parsed->validate(); !value) {
            reply(res, serde::Ser::serialize_error(accept, value.error()),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            return;
        }

        m_ctx.get().get_connector().update<model::WorkflowDef>(*parsed, [&](bool oke) noexcept {
            if (!oke) {
                reply(res, serde::Ser::serialize_error(accept, "not found"),
                      interfaces::io::types::Status::NOT_FOUND);
                return;
            }
            reply(res, serde::Ser::serialize(accept, *parsed));
        });
    }

    void remove_definition(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto name = std::string{target.substr(target.rfind('/') + 1)};

        m_ctx.get().get_connector().remove<model::WorkflowDef>(name, [&](bool oke) noexcept {
            if (!oke) {
                reply(res, serde::Ser::serialize_error(accept, "not found"),
                      interfaces::io::types::Status::NOT_FOUND);
                return;
            }
            res.set_status(interfaces::io::types::Status::NO_CONTENT);
        });
    }

    // Path: /api/v1/workflows/:name/start — def_name is the segment before "start".
    void start_execution(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto target = req.get_path();
        auto last = target.rfind('/');
        auto before = target.rfind('/', last > 0 ? last - 1 : 0);
        auto def_name = std::string{target.substr(before + 1, last - before - 1)};

        std::unordered_map<std::string, std::string> variables;
        auto body = flatten_body(req);
        if (!body.empty()) {
            if (auto parsed = serde::Ser::deserialize<WorkflowStartBody>(content_type, body)) {
                variables = parsed->get_variables();
            }
        }

        model::WorkflowExecution exec;
        exec.set_exec_id(model::generate_id());
        exec.set_def_name(def_name);
        exec.set_status(model::WorkflowStatus::RUNNING);
        exec.set_variables(std::move(variables));

        model::ExecutionTimings timings;
        timings.set_started_at(std::chrono::system_clock::now());
        exec.set_timings(timings);

        m_ctx.get().get_connector().insert<model::WorkflowExecution>(exec, [&](bool oke) noexcept {
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
    void get_execution(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto exec_id_str = std::string{target.substr(target.rfind('/') + 1)};

        m_ctx.get().get_connector().find<model::WorkflowExecution>(
            exec_id_str, [&](std::optional<model::WorkflowExecution> result) noexcept {
                if (!result) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    return;
                }
                reply(res, serde::Ser::serialize(accept, *result));
            });
    }

    // Path: /api/v1/workflows/exec/:id — exec_id is the last segment.
    void terminate_execution(interfaces::io::IRequest &req,
                             interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto exec_id_str = std::string{target.substr(target.rfind('/') + 1)};

        std::optional<model::WorkflowExecution> found;
        bool handled = false;
        m_ctx.get().get_connector().find<model::WorkflowExecution>(
            exec_id_str, [&](std::optional<model::WorkflowExecution> result) noexcept {
                if (!result) {
                    core::logger::warning("engine", "wf/terminate not found: '{}'", exec_id_str);
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    handled = true;
                    return;
                }
                if (model::is_terminal(result->get_status())) {
                    core::logger::warning("engine", "wf/terminate already terminal: '{}'",
                                          exec_id_str);
                    reply(res, serde::Ser::serialize_error(accept, "already in terminal state"),
                          interfaces::io::types::Status::CONFLICT);
                    handled = true;
                    return;
                }
                found = std::move(result);
            });

        if (handled) {
            return;
        }

        found->set_status(model::WorkflowStatus::TERMINATED);
        m_ctx.get().get_connector().update<model::WorkflowExecution>(
            *found, [&](bool oke) noexcept {
                if (!oke) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    return;
                }
                core::logger::info("engine", "exec terminated: '{}'", exec_id_str);
                reply(res, serde::Ser::serialize(accept, *found));
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

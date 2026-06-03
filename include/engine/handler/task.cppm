module;
#include <rfl/json.hpp>

export module engine:task;

import std;
import interfaces;
import model;
import shared;
import serde;
import :context;

// ─── TaskSubmitBody ───────────────────────────────────────────────────────────

namespace engine {

class TaskSubmitBody {
  public:
    void set_result(model::TaskResult r) noexcept { m_result = r; }
    void set_output_data(std::unordered_map<std::string, std::string> d) noexcept {
        m_output_data = std::move(d);
    }
    [[nodiscard]] model::TaskResult get_result() const noexcept { return m_result; }
    [[nodiscard]] const std::unordered_map<std::string, std::string> &
    get_output_data() const noexcept {
        return m_output_data;
    }

  private:
    model::TaskResult m_result{model::TaskResult::SUCCESS};
    std::unordered_map<std::string, std::string> m_output_data;
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

// ─── TaskHandler ──────────────────────────────────────────────────────────────

export namespace engine {

// Routes:
//   GET    /api/v1/tasks/:name         → get_definition
//   POST   /api/v1/tasks               → create_definition
//   PUT    /api/v1/tasks/:name         → update_definition
//   DELETE /api/v1/tasks/:name         → remove_definition
//   GET    /api/v1/tasks/queue/:type   → poll
//   POST   /api/v1/tasks/:id/result    → submit_result
template <typename Protocol>
class TaskHandler {
  public:
    explicit TaskHandler(EngineContext &ctx) noexcept : m_ctx{ctx} {}

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

        m_ctx.get().get_task_def_connector().find(
            name, [&](std::optional<model::TaskDef> result) noexcept {
                if (!result) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::Status::NOT_FOUND);
                    return;
                }
                reply(res, serde::Ser::serialize(accept, *result));
            });
        m_ctx.get().get_task_def_connector().flush();
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
        auto parsed = serde::Ser::deserialize<model::TaskDef>(content_type, body);
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

        m_ctx.get().get_task_def_connector().insert(*parsed, [&](bool ok) noexcept {
            if (!ok) {
                reply(res, serde::Ser::serialize_error(accept, "insert failed"),
                      interfaces::Status::INTERNAL_SERVER_ERROR);
                return;
            }
            reply(res, serde::Ser::serialize(accept, *parsed), interfaces::Status::CREATED);
        });
        m_ctx.get().get_task_def_connector().flush();
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
        auto parsed = serde::Ser::deserialize<model::TaskDef>(content_type, body);
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

        m_ctx.get().get_task_def_connector().update(*parsed, [&](bool ok) noexcept {
            if (!ok) {
                reply(res, serde::Ser::serialize_error(accept, "not found"),
                      interfaces::Status::NOT_FOUND);
                return;
            }
            reply(res, serde::Ser::serialize(accept, *parsed));
        });
        m_ctx.get().get_task_def_connector().flush();
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

        m_ctx.get().get_task_def_connector().remove(name, [&](bool ok) noexcept {
            if (!ok) {
                reply(res, serde::Ser::serialize_error(accept, "not found"),
                      interfaces::Status::NOT_FOUND);
                return;
            }
            res.set_status(interfaces::Status::NO_CONTENT);
        });
        m_ctx.get().get_task_def_connector().flush();
    }

    void poll(interfaces::IRequest<Protocol> &req, interfaces::IResponse<Protocol> &res) noexcept {
        auto accept = req.find_header("accept");
        auto target = req.get_target();
        auto worker_type = std::string{target.substr(target.rfind('/') + 1)};

        if (!m_ctx.get().get_db()) {
            reply(res, serde::Ser::serialize_error(accept, "service unavailable"),
                  interfaces::Status::SERVICE_UNAVAILABLE);
            return;
        }

        // Atomically claim the oldest SCHEDULED task for this worker_type.
        // FOR UPDATE SKIP LOCKED prevents double-polling under concurrency.
        auto sql = std::format("WITH claimed AS ("
                               "UPDATE task_instances "
                               "SET status = 'IN_PROGRESS' "
                               "WHERE task_id = ("
                               "SELECT ti.task_id FROM task_instances ti "
                               "JOIN task_definitions td ON ti.def_name = td.name "
                               "WHERE ti.status = 'SCHEDULED' AND td.worker_type = '{}' "
                               "ORDER BY ti.seq ASC LIMIT 1 FOR UPDATE SKIP LOCKED"
                               ") RETURNING *"
                               ") SELECT row_to_json(claimed) FROM claimed",
                               worker_type);

        m_ctx.get().get_db()->query(sql, [&](std::string_view result) noexcept {
            if (result.empty()) {
                res.set_status(interfaces::Status::NO_CONTENT);
                return;
            }
            auto v = serde::Json::decode<model::TaskInstance>(result);
            if (!v) {
                reply(res, serde::Ser::serialize_error(accept, "decode error"),
                      interfaces::Status::INTERNAL_SERVER_ERROR);
                return;
            }
            reply(res, serde::Ser::serialize(accept, *v));
        });
    }

    void submit_result(interfaces::IRequest<Protocol> &req,
                       interfaces::IResponse<Protocol> &res) noexcept {
        // Path: /api/v1/tasks/:id/result — task_id is the segment before "result".
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto target = req.get_target();
        auto last = target.rfind('/');
        auto before = target.rfind('/', last > 0 ? last - 1 : 0);
        auto task_id = std::string{target.substr(before + 1, last - before - 1)};

        if (!m_ctx.get().get_db()) {
            reply(res, serde::Ser::serialize_error(accept, "service unavailable"),
                  interfaces::Status::SERVICE_UNAVAILABLE);
            return;
        }

        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<TaskSubmitBody>(content_type, body);
        if (!parsed) {
            reply(res, serde::Ser::serialize_error(accept, "invalid body"),
                  interfaces::Status::BAD_REQUEST);
            return;
        }

        constexpr auto result_to_status = [](model::TaskResult r) noexcept -> std::string_view {
            switch (r) {
            case model::TaskResult::SUCCESS:
                return "COMPLETED";
            case model::TaskResult::FAILURE:
                return "FAILED";
            case model::TaskResult::TIMEOUT:
                return "TIMED_OUT";
            case model::TaskResult::SKIPPED:
                return "SKIPPED";
            }
            return "FAILED";
        };

        // output_data goes to a JSONB column — always encode as JSON for the DB.
        auto output_json = rfl::json::write(parsed->get_output_data());
        auto sql = std::format("UPDATE task_instances "
                               "SET status = '{}', output_data = '{}'::jsonb "
                               "WHERE task_id = '{}' RETURNING row_to_json(task_instances.*)",
                               result_to_status(parsed->get_result()), output_json, task_id);

        m_ctx.get().get_db()->query(sql, [&](std::string_view result) noexcept {
            if (result.empty()) {
                reply(res, serde::Ser::serialize_error(accept, "not found"),
                      interfaces::Status::NOT_FOUND);
                return;
            }
            auto v = serde::Json::decode<model::TaskInstance>(result);
            if (!v) {
                reply(res, serde::Ser::serialize_error(accept, "decode error"),
                      interfaces::Status::INTERNAL_SERVER_ERROR);
                return;
            }
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

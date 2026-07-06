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
    void set_result(model::TaskResult result) noexcept { m_result = result; }
    void set_output_data(std::unordered_map<std::string, std::string> data) noexcept {
        m_output_data = std::move(data);
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
class TaskHandler {
  public:
    explicit TaskHandler(EngineContext &ctx) noexcept : m_ctx{ctx} {}

    void get_definition(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto name = std::string{target.substr(target.rfind('/') + 1)};

        m_ctx.get().get_connector().find<model::TaskDef>(
            name, [&](std::optional<model::TaskDef> result) noexcept {
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
        auto parsed = serde::Ser::deserialize<model::TaskDef>(content_type, body);
        if (!parsed) {
            core::logger::warning("engine", "task/create bad request: {}", parsed.error());
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            return;
        }

        if (auto validate = parsed->validate(); !validate) {
            core::logger::warning("engine", "task/create invalid: {}", validate.error());
            reply(res, serde::Ser::serialize_error(accept, validate.error()),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            return;
        }

        m_ctx.get().get_connector().insert<model::TaskDef>(*parsed, [&](bool oke) noexcept {
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

    void update_definition(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");

        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::TaskDef>(content_type, body);
        if (!parsed) {
            core::logger::warning("engine", "task/update bad request: {}", parsed.error());
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            return;
        }

        if (auto validate = parsed->validate(); !validate) {
            core::logger::warning("engine", "task/update invalid: {}", validate.error());
            reply(res, serde::Ser::serialize_error(accept, validate.error()),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            return;
        }

        m_ctx.get().get_connector().update<model::TaskDef>(*parsed, [&](bool oke) noexcept {
            if (!oke) {
                core::logger::warning("engine", "task/update not found: '{}'", parsed->get_name());
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

        m_ctx.get().get_connector().remove<model::TaskDef>(name, [&](bool oke) noexcept {
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

    void poll(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto worker_type = std::string{target.substr(target.rfind('/') + 1)};

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
            [](const model::TaskInstance &lhs, const model::TaskInstance &rhs) noexcept {
                return lhs.get_seq() < rhs.get_seq();
            },
            [&, accept](std::optional<model::TaskInstance> found) mutable noexcept {
                if (!found) {
                    res.set_status(interfaces::io::types::Status::NO_CONTENT);
                    return;
                }
                found->set_status(model::TaskStatus::IN_PROGRESS);
                auto claimed = std::move(*found);
                m_ctx.get().get_connector().update<model::TaskInstance>(
                    claimed, [&res, accept, claimed](bool oke) mutable noexcept {
                        if (!oke) {
                            reply(res, serde::Ser::serialize_error(accept, "claim failed"),
                                  interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
                            return;
                        }
                        reply(res, serde::Ser::serialize(accept, claimed));
                    });
            });
    }

    void submit_result(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto target = req.get_path();
        auto last = target.rfind('/');
        auto before = target.rfind('/', last > 0 ? last - 1 : 0);
        auto task_id = std::string{target.substr(before + 1, last - before - 1)};

        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<TaskSubmitBody>(content_type, body);
        if (!parsed) {
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            return;
        }

        m_ctx.get().get_connector().find<model::TaskInstance>(
            task_id, [&, accept, submit = std::move(*parsed)](
                         std::optional<model::TaskInstance> found) mutable noexcept {
                if (!found) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    return;
                }

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

                found->set_status(TO_STATUS(submit.get_result()));
                found->set_output_data(submit.get_output_data());
                auto updated = std::move(*found);

                m_ctx.get().get_connector().update<model::TaskInstance>(
                    updated, [&res, accept, updated](bool oke) mutable noexcept {
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

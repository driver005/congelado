export module engine:schedule_handler;

import std;
import interfaces;
import model;
import shared;
import serde;
import core_logger;
import core_events;
import :context;
import :cron;

export namespace engine {

/// @brief One entry in the response to `GET /api/v1/schedules/:name/next-few-runs`.
class ScheduleNextRun {
  public:
    ScheduleNextRun() = default;
    explicit ScheduleNextRun(std::chrono::system_clock::time_point at) noexcept : m_at{at} {}
    void set_at(std::chrono::system_clock::time_point at) noexcept { m_at = at; }
    [[nodiscard]] std::chrono::system_clock::time_point get_at() const noexcept { return m_at; }

  private:
    std::chrono::system_clock::time_point m_at;
};

// Routes:
//   GET    /api/v1/schedules                       → list_schedules
//   POST   /api/v1/schedules                       → create_schedule
//   GET    /api/v1/schedules/:name                 → get_schedule
//   PUT    /api/v1/schedules/:name                 → update_schedule
//   DELETE /api/v1/schedules/:name                 → remove_schedule
//   POST   /api/v1/schedules/:name/pause           → pause_schedule
//   POST   /api/v1/schedules/:name/resume          → resume_schedule
//   GET    /api/v1/schedules/:name/next_few_runs   → next_few_runs (fixed count of 5 — see its
//                                                     own docs for why this isn't a `?count=`
//                                                     query param)
class ScheduleHandler {
  public:
    explicit ScheduleHandler(EngineContext &ctx) noexcept : m_ctx{ctx} {}

    void list_schedules(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");
        m_ctx.get().get_connector().find_all<model::WorkflowSchedule>(
            [&](const std::vector<model::WorkflowSchedule> &schedules) {
                reply(res, serde::Ser::serialize(accept, schedules));
            });
    }

    void get_schedule(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");
        auto name = last_segment(req);
        m_ctx.get().get_connector().find<model::WorkflowSchedule>(
            name, [&](std::optional<model::WorkflowSchedule> result) {
                if (!result) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    return;
                }
                reply(res, serde::Ser::serialize(accept, *result));
            });
    }

    void create_schedule(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::WorkflowSchedule>(content_type, body);
        if (!parsed) {
            core::logger::warning("engine", "schedule/create bad request: {}", parsed.error());
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            return;
        }
        if (auto validate = parsed->validate(); !validate) {
            reply(res, serde::Ser::serialize_error(accept, validate.error()),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            return;
        }
        if (!CronExpression::parse(parsed->get_cron_expression())) {
            reply(res, serde::Ser::serialize_error(accept, "cron_expression does not parse"),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            return;
        }
        m_ctx.get().get_connector().upsert<model::WorkflowSchedule>(*parsed, [&](bool oke) {
            if (!oke) {
                reply(res, serde::Ser::serialize_error(accept, "upsert failed"),
                      interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
                return;
            }
            core::logger::info("engine", "schedule created: '{}'", parsed->get_name());
            core::events::publish("engine.schedule.created", {{"name", parsed->get_name()}});
            reply(res, serde::Ser::serialize(accept, *parsed),
                  interfaces::io::types::Status::CREATED);
        });
    }

    void update_schedule(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::WorkflowSchedule>(content_type, body);
        if (!parsed) {
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            return;
        }
        if (auto validate = parsed->validate(); !validate) {
            reply(res, serde::Ser::serialize_error(accept, validate.error()),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            return;
        }
        m_ctx.get().get_connector().update<model::WorkflowSchedule>(*parsed, [&](bool oke) {
            if (!oke) {
                reply(res, serde::Ser::serialize_error(accept, "not found"),
                      interfaces::io::types::Status::NOT_FOUND);
                return;
            }
            reply(res, serde::Ser::serialize(accept, *parsed));
        });
    }

    void remove_schedule(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");
        auto name = last_segment(req);
        m_ctx.get().get_connector().remove<model::WorkflowSchedule>(name, [&](bool oke) {
            if (!oke) {
                reply(res, serde::Ser::serialize_error(accept, "not found"),
                      interfaces::io::types::Status::NOT_FOUND);
                return;
            }
            res.set_status(interfaces::io::types::Status::NO_CONTENT);
        });
    }

    void pause_schedule(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
        set_paused(req, res, true);
    }

    void resume_schedule(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
        set_paused(req, res, false);
    }

    /**
     * @brief Handles `GET /api/v1/schedules/:name/next_few_runs` — the next 5 times this
     * schedule's cron expression would fire, computed from now (not from last_fired_at, so this
     * reflects "if it fired right now" rather than the real next-fire accounting for a pending
     * catch-up — a preview, not a guarantee).
     * @warning Fixed at 5 runs, not a `?count=` query param — IRequest has no query-string
     * parsing anywhere in this codebase to build on (same reasoning as EventHandlerHandler's
     * list_handlers() skipping `?event=` filtering).
     * @param req the inbound request; path supplies the schedule name.
     * @param res the response — 200 with up to 5 upcoming times, 404 if the schedule wasn't
     * found, 422 if its cron_expression doesn't parse.
     */
    void next_few_runs(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto last = target.rfind('/');
        auto before = target.rfind('/', last > 0 ? last - 1 : 0);
        auto name = std::string{target.substr(before + 1, last - before - 1)};

        m_ctx.get().get_connector().find<model::WorkflowSchedule>(
            name, [&](std::optional<model::WorkflowSchedule> schedule) {
                if (!schedule) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    return;
                }
                auto cron = CronExpression::parse(schedule->get_cron_expression());
                if (!cron) {
                    reply(res, serde::Ser::serialize_error(accept, "cron_expression does not parse"),
                          interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
                    return;
                }
                std::vector<ScheduleNextRun> runs;
                auto cursor = std::chrono::system_clock::now();
                for (int i = 0; i < 5; ++i) {
                    auto next = cron->next_after(cursor);
                    if (!next) {
                        break;
                    }
                    runs.emplace_back(*next);
                    cursor = *next;
                }
                reply(res, serde::Ser::serialize(accept, runs));
            });
    }

  private:
    std::reference_wrapper<EngineContext> m_ctx;

    void set_paused(interfaces::io::IRequest &req, interfaces::io::IResponse &res, bool paused) {
        auto accept = req.find_header("accept");
        auto name = second_to_last_segment(req);
        m_ctx.get().get_connector().find<model::WorkflowSchedule>(
            name, [this, &res, accept, paused](std::optional<model::WorkflowSchedule> schedule) {
                if (!schedule) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    return;
                }
                schedule->set_paused(paused);
                m_ctx.get().get_connector().update<model::WorkflowSchedule>(
                    *schedule, [&res, accept](bool oke) {
                        if (!oke) {
                            reply(res, serde::Ser::serialize_error(accept, "not found"),
                                  interfaces::io::types::Status::NOT_FOUND);
                            return;
                        }
                        res.set_status(interfaces::io::types::Status::OK);
                    });
            });
    }

    static std::string last_segment(interfaces::io::IRequest &req) {
        auto target = req.get_path();
        return std::string{target.substr(target.rfind('/') + 1)};
    }

    static std::string second_to_last_segment(interfaces::io::IRequest &req) {
        auto target = req.get_path();
        auto last = target.rfind('/');
        auto before = target.rfind('/', last > 0 ? last - 1 : 0);
        return std::string{target.substr(before + 1, last - before - 1)};
    }

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

template <>
struct serde::Serializable<engine::ScheduleNextRun> {
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"at", &engine::ScheduleNextRun::get_at,
                             &engine::ScheduleNextRun::set_at>{},
        };
    }
};

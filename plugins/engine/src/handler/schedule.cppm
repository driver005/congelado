export module engine:schedule_handler;

import std;
import interfaces;
import model;
import shared;
import serde;
import core_logger;
import core_events;
import :context;

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

    void list_schedules(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                        std::function<void()> send) noexcept {
        auto accept = req.find_header("accept");
        m_ctx.get().get_connector().find_all<model::WorkflowSchedule>(
            [&res, accept,
             send = std::move(send)](const std::vector<model::WorkflowSchedule> &schedules) {
                reply(res, serde::Ser::serialize(accept, schedules));
                send();
            });
    }

    void get_schedule(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                      std::function<void()> send) noexcept {
        auto accept = req.find_header("accept");
        auto name = last_segment(req);
        m_ctx.get().get_connector().find<model::WorkflowSchedule>(
            name, [&res, accept,
                   send = std::move(send)](std::optional<model::WorkflowSchedule> result) {
                if (!result) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    send();
                    return;
                }
                reply(res, serde::Ser::serialize(accept, *result));
                send();
            });
    }

    void create_schedule(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                         std::function<void()> send) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::WorkflowSchedule>(content_type, body);
        if (!parsed) {
            core::logger::warning("engine", "schedule/create bad request: {}", parsed.error());
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            send();
            return;
        }
        if (auto validate = parsed->validate(); !validate) {
            reply(res, serde::Ser::serialize_error(accept, validate.error()),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            send();
            return;
        }
        auto *cron = m_ctx.get().get_cron();
        if (cron == nullptr) {
            reply(res, serde::Ser::serialize_error(accept, "no cron backend configured"),
                  interfaces::io::types::Status::SERVICE_UNAVAILABLE);
            send();
            return;
        }
        if (!cron->validate(parsed->get_cron_expression())) {
            reply(res, serde::Ser::serialize_error(accept, "cron_expression does not parse"),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            send();
            return;
        }
        model::WorkflowSchedule schedule = *parsed;
        m_ctx.get().get_connector().upsert<model::WorkflowSchedule>(
            schedule, [&res, accept, schedule, cron, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(res, serde::Ser::serialize_error(accept, "upsert failed"),
                          interfaces::io::types::Status::INTERNAL_SERVER_ERROR);
                    send();
                    return;
                }
                if (schedule.get_enabled() && !schedule.get_paused()) {
                    cron->upsert_job(schedule.get_name(), schedule.get_cron_expression());
                }
                core::logger::info("engine", "schedule created: '{}'", schedule.get_name());
                core::events::publish("engine.schedule.created", {{"name", schedule.get_name()}});
                reply(res, serde::Ser::serialize(accept, schedule),
                      interfaces::io::types::Status::CREATED);
                send();
            });
    }

    void update_schedule(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                         std::function<void()> send) {
        auto accept = req.find_header("accept");
        auto content_type = req.find_header("content-type");
        auto body = flatten_body(req);
        auto parsed = serde::Ser::deserialize<model::WorkflowSchedule>(content_type, body);
        if (!parsed) {
            reply(res, serde::Ser::serialize_error(accept, parsed.error()),
                  interfaces::io::types::Status::BAD_REQUEST);
            send();
            return;
        }
        if (auto validate = parsed->validate(); !validate) {
            reply(res, serde::Ser::serialize_error(accept, validate.error()),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            send();
            return;
        }
        auto *cron = m_ctx.get().get_cron();
        if (cron == nullptr) {
            reply(res, serde::Ser::serialize_error(accept, "no cron backend configured"),
                  interfaces::io::types::Status::SERVICE_UNAVAILABLE);
            send();
            return;
        }
        if (!cron->validate(parsed->get_cron_expression())) {
            reply(res, serde::Ser::serialize_error(accept, "cron_expression does not parse"),
                  interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
            send();
            return;
        }
        model::WorkflowSchedule schedule = *parsed;
        m_ctx.get().get_connector().update<model::WorkflowSchedule>(
            schedule, [&res, accept, schedule, cron, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    send();
                    return;
                }
                // Keep the cron backend in sync with the schedule's new state — a now-disabled or
                // paused schedule drops out of the backend, an active one is (re)registered.
                if (schedule.get_enabled() && !schedule.get_paused()) {
                    cron->upsert_job(schedule.get_name(), schedule.get_cron_expression());
                } else {
                    cron->remove_job(schedule.get_name());
                }
                reply(res, serde::Ser::serialize(accept, schedule));
                send();
            });
    }

    void remove_schedule(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                         std::function<void()> send) noexcept {
        auto accept = req.find_header("accept");
        auto name = last_segment(req);
        auto *cron = m_ctx.get().get_cron();
        m_ctx.get().get_connector().remove<model::WorkflowSchedule>(
            name, [&res, accept, name, cron, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    send();
                    return;
                }
                if (cron != nullptr) {
                    cron->remove_job(name);
                }
                res.set_status(interfaces::io::types::Status::NO_CONTENT);
                send();
            });
    }

    void pause_schedule(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                        std::function<void()> send) {
        set_paused(req, res, std::move(send), true);
    }

    void resume_schedule(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                         std::function<void()> send) {
        set_paused(req, res, std::move(send), false);
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
    void next_few_runs(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                       std::function<void()> send) {
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto last = target.rfind('/');
        auto before = target.rfind('/', last > 0 ? last - 1 : 0);
        auto name = std::string{target.substr(before + 1, last - before - 1)};

        auto *cron = m_ctx.get().get_cron();
        if (cron == nullptr) {
            reply(res, serde::Ser::serialize_error(accept, "no cron backend configured"),
                  interfaces::io::types::Status::SERVICE_UNAVAILABLE);
            send();
            return;
        }
        m_ctx.get().get_connector().find<model::WorkflowSchedule>(
            name, [&res, accept, cron,
                   send = std::move(send)](std::optional<model::WorkflowSchedule> schedule) {
                if (!schedule) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    send();
                    return;
                }
                if (!cron->validate(schedule->get_cron_expression())) {
                    reply(res,
                          serde::Ser::serialize_error(accept, "cron_expression does not parse"),
                          interfaces::io::types::Status::UNPROCESSABLE_CONTENT);
                    send();
                    return;
                }
                std::vector<ScheduleNextRun> runs;
                auto cursor = std::chrono::system_clock::now();
                for (int i = 0; i < 5; ++i) {
                    auto next = cron->next_after(schedule->get_cron_expression(), cursor);
                    if (!next) {
                        break;
                    }
                    runs.emplace_back(*next);
                    cursor = *next;
                }
                reply(res, serde::Ser::serialize(accept, runs));
                send();
            });
    }

  private:
    std::reference_wrapper<EngineContext> m_ctx;

    void set_paused(interfaces::io::IRequest &req, interfaces::io::IResponse &res,
                    std::function<void()> send, bool paused) {
        auto accept = req.find_header("accept");
        auto name = second_to_last_segment(req);
        auto *cron = m_ctx.get().get_cron();
        m_ctx.get().get_connector().find<model::WorkflowSchedule>(
            name, [this, &res, accept, paused, cron,
                   send = std::move(send)](std::optional<model::WorkflowSchedule> schedule) {
                if (!schedule) {
                    reply(res, serde::Ser::serialize_error(accept, "not found"),
                          interfaces::io::types::Status::NOT_FOUND);
                    send();
                    return;
                }
                schedule->set_paused(paused);
                model::WorkflowSchedule updated = *schedule;
                m_ctx.get().get_connector().update<model::WorkflowSchedule>(
                    updated, [&res, accept, updated, cron, send = std::move(send)](bool oke) {
                        if (!oke) {
                            reply(res, serde::Ser::serialize_error(accept, "not found"),
                                  interfaces::io::types::Status::NOT_FOUND);
                            send();
                            return;
                        }
                        if (cron != nullptr) {
                            if (updated.get_enabled() && !updated.get_paused()) {
                                cron->upsert_job(updated.get_name(),
                                                 updated.get_cron_expression());
                            } else {
                                cron->remove_job(updated.get_name());
                            }
                        }
                        res.set_status(interfaces::io::types::Status::OK);
                        send();
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

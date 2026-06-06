export module worker:poll;

import std;
import interfaces;
import model;
import serde;
import core_logger;
import :context;

export namespace worker {

// Routes registered by PollHandler<Protocol>:
//
//   POST   /api/v1/worker/poll/:type      → poll
//   POST   /api/v1/worker/ack/:id         → ack
//
// Usage:
//   PollHandler<Protocol>::bind(worker_ctx);
//   // then register static methods as HandlerFn<Protocol> in RouterContext
//
// call_engine() blocks the calling contract thread via std::future::get() while
// EngineClient's receive contract runs on a separate pool thread. Requires the
// ContractThreadPool to have at least 2 threads (default: hardware_concurrency()).
template <typename Protocol>
class PollHandler {
  public:
    // Inject identity before the first poll is issued.
    static void bind(WorkerContext &ctx) noexcept { s_ctx = &ctx; }

    // POST /api/v1/worker/poll/:type
    // 1. Parse :type, look up registered task worker.
    // 2. Poll engine GET /api/v1/tasks/queue/:type.
    // 3. On 204 (empty queue): respond NO_CONTENT.
    // 4. On 200: deserialise TaskInstance → TaskInput, execute task, submit result.
    static void poll(interfaces::IRequest<Protocol> &req,
                     interfaces::IResponse<Protocol> &res) noexcept {
        auto target = req.get_target();
        auto type = std::string{target.substr(target.rfind('/') + 1)};

        if (s_ctx->get_task_worker(type) == nullptr) {
            res.set_status(interfaces::Status::BAD_REQUEST);
            return;
        }

        s_ctx->call_engine("GET", "/api/v1/tasks/queue/" + type, "",
            [&res, type](int status, std::string body) noexcept {
                if (status == 204) {
                    res.set_status(interfaces::Status::NO_CONTENT);
                    return;
                }
                if (status != 200) {
                    core::logger::error("worker/poll", "engine poll failed status={}", status);
                    res.set_status(interfaces::Status::INTERNAL_SERVER_ERROR);
                    return;
                }

                auto parsed = serde::Json::decode<model::TaskInstance>(body);
                if (!parsed.has_value()) {
                    core::logger::error("worker/poll", "parse TaskInstance failed: {}", parsed.error());
                    res.set_status(interfaces::Status::INTERNAL_SERVER_ERROR);
                    return;
                }

                auto &instance = *parsed;
                TaskInput input{instance.get_input_data()};
                auto output_opt = s_ctx->run_task(instance.get_def_name(), input);

                auto result = output_opt.has_value() ? model::TaskResult::SUCCESS
                                                     : model::TaskResult::FAILURE;
                const auto &output_data = output_opt ? output_opt->get_data()
                                                     : std::unordered_map<std::string, std::string>{};

                auto task_id = std::format("{}", instance.get_task_id());
                auto submit_path = "/api/v1/tasks/" + task_id + "/result";
                auto submit_body = build_submit_json(result, output_data);

                s_ctx->call_engine("POST", submit_path, submit_body,
                    [&res](int submit_status, std::string) noexcept {
                        if (submit_status == 200) {
                            res.set_status(interfaces::Status::OK);
                        } else {
                            core::logger::error("worker/poll", "submit result failed status={}", submit_status);
                            res.set_status(interfaces::Status::INTERNAL_SERVER_ERROR);
                        }
                    });
            });
    }

    // POST /api/v1/worker/ack/:id
    // PATCH /api/v1/tasks/:id/heartbeat on engine — resets timeout clock.
    static void ack(interfaces::IRequest<Protocol> &req,
                    interfaces::IResponse<Protocol> &res) noexcept {
        auto target = req.get_target();
        auto task_id = std::string{target.substr(target.rfind('/') + 1)};

        s_ctx->call_engine("PATCH", "/api/v1/tasks/" + task_id + "/heartbeat", "",
            [&res](int status, std::string) noexcept {
                if (status == 200) {
                    res.set_status(interfaces::Status::OK);
                } else if (status == 404) {
                    res.set_status(interfaces::Status::NOT_FOUND);
                } else {
                    res.set_status(interfaces::Status::INTERNAL_SERVER_ERROR);
                }
            });
    }

  private:
    static std::string build_submit_json(model::TaskResult result,
                                          const std::unordered_map<std::string, std::string> &data) noexcept {
        std::string_view result_str;
        switch (result) {
        case model::TaskResult::SUCCESS: result_str = "SUCCESS"; break;
        case model::TaskResult::FAILURE: result_str = "FAILURE"; break;
        case model::TaskResult::TIMEOUT: result_str = "TIMEOUT"; break;
        case model::TaskResult::SKIPPED: result_str = "SKIPPED"; break;
        }

        std::string json = std::format("{{\"result\":\"{}\",\"output_data\":{{", result_str);
        bool first = true;
        for (const auto &[key, value] : data) {
            if (!first) json += ',';
            json += std::format("\"{}\":\"{}\"", key, value);
            first = false;
        }
        json += "}}";
        return json;
    }

    static inline WorkerContext *s_ctx{nullptr};
};

} // namespace worker

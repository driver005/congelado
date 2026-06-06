export module worker:execution;

import std;
import interfaces;
import model;
import serde;
import core_logger;
import :context;

export namespace worker {

// Routes registered by ExecutionHandler<Protocol>:
//
//   GET    /api/v1/worker/executions           → list_executions
//   GET    /api/v1/worker/executions/:id       → get_execution
//   DELETE /api/v1/worker/executions/:id       → cancel_execution
//
// Usage:
//   ExecutionHandler<Protocol>::bind(worker_ctx);
//   // then register static methods as HandlerFn<Protocol> in RouterContext
template <typename Protocol>
class ExecutionHandler {
  public:
    // Inject identity before the first request arrives.
    static void bind(WorkerContext &ctx) noexcept { s_ctx = &ctx; }

    // GET /api/v1/worker/executions
    // Queries engine for IN_PROGRESS tasks owned by this worker, forwards JSON.
    static void list_executions(interfaces::IRequest<Protocol> &req,
                                interfaces::IResponse<Protocol> &res) noexcept {
        auto accept = req.find_header("accept");
        auto worker_id = std::string{s_ctx->get_worker_id()};
        auto path = "/api/v1/tasks?worker_id=" + worker_id + "&status=IN_PROGRESS";

        s_ctx->call_engine("GET", path, "",
            [&res, accept](int status, std::string body) noexcept {
                if (status == 200) {
                    reply(res, bytes_from(body));
                } else {
                    core::logger::error("worker/executions", "list failed status={}", status);
                    res.set_status(interfaces::Status::INTERNAL_SERVER_ERROR);
                }
            });
    }

    // GET /api/v1/worker/executions/:id
    // Queries engine for task :id, forwards result.
    // Note: TaskInstance has no worker_id field yet — ownership check skipped.
    static void get_execution(interfaces::IRequest<Protocol> &req,
                              interfaces::IResponse<Protocol> &res) noexcept {
        auto accept = req.find_header("accept");
        auto target = req.get_target();
        auto task_id = std::string{target.substr(target.rfind('/') + 1)};

        s_ctx->call_engine("GET", "/api/v1/tasks/" + task_id, "",
            [&res, accept](int status, std::string body) noexcept {
                if (status == 404) {
                    res.set_status(interfaces::Status::NOT_FOUND);
                    return;
                }
                if (status != 200) {
                    core::logger::error("worker/executions", "get {} failed status={}", body, status);
                    res.set_status(interfaces::Status::INTERNAL_SERVER_ERROR);
                    return;
                }
                reply(res, bytes_from(body));
            });
    }

    // DELETE /api/v1/worker/executions/:id
    // Forwards cancel to engine DELETE /api/v1/tasks/:id.
    static void cancel_execution(interfaces::IRequest<Protocol> &req,
                                 interfaces::IResponse<Protocol> &res) noexcept {
        auto target = req.get_target();
        auto task_id = std::string{target.substr(target.rfind('/') + 1)};

        s_ctx->call_engine("DELETE", "/api/v1/tasks/" + task_id, "",
            [&res](int status, std::string) noexcept {
                if (status == 200 || status == 204) {
                    res.set_status(interfaces::Status::NO_CONTENT);
                } else if (status == 404) {
                    res.set_status(interfaces::Status::NOT_FOUND);
                } else {
                    res.set_status(interfaces::Status::INTERNAL_SERVER_ERROR);
                }
            });
    }

  private:
    static void reply(interfaces::IResponse<Protocol> &res, std::vector<std::byte> bytes,
                      interfaces::Status status = interfaces::Status::OK) noexcept {
        res.set_body(std::move(bytes));
        res.set_status(status);
    }

    static std::vector<std::byte> bytes_from(const std::string &text) noexcept {
        std::vector<std::byte> result;
        result.reserve(text.size());
        for (char character : text) {
            result.push_back(static_cast<std::byte>(character));
        }
        return result;
    }

    static inline WorkerContext *s_ctx{nullptr};
};

} // namespace worker

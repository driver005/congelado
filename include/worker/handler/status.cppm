export module worker:status;

import std;
import interfaces;
import :context;

export namespace worker {

// Routes registered by StatusHandler :
//
//   GET /api/v1/worker/health    → health_check   ← fully implemented
//   GET /api/v1/worker/info      → worker_info
//
// Usage:
//   StatusHandler::bind(worker_ctx);
//   // then register static methods as HandlerFn  in RouterContext
class StatusHandler {
  public:
    // Inject identity before the first request arrives.
    // static void bind(WorkerContext &ctx) noexcept { s_ctx = &ctx; }

    // Workers are stateless — no DB probe, no cache. Always returns ok.
    static void health_check(interfaces::io::IRequest & /*req*/,
                             interfaces::io::IResponse &res) noexcept {
        static constexpr std::string_view k_ok = R"({"status":"ok"})";

        std::vector<std::byte> bytes(k_ok.size());
        std::ranges::transform(k_ok, bytes.begin(), [](char ch) noexcept { return std::byte(ch); });
        res.set_body(std::move(bytes));
        res.set_status(interfaces::io::types::Status::OK);
    }

    // TODO: build JSON from WorkerContext fields
    //       {"worker_id":"...","task_types":[...],"status":"active"}
    static void worker_info(interfaces::io::IRequest &req,
                            interfaces::io::IResponse &res) noexcept {
        res.set_status(interfaces::io::types::Status::NOT_IMPLEMENTED);
    }

    // private:
    //   static inline WorkerContext *s_ctx{nullptr};
};

} // namespace worker

export module engine:metadata_handler;

import std;
import interfaces;
import model;
import shared;
import :context;

export namespace engine {

// Routes registered by MetadataHandler<Protocol>:
//
//   GET /api/v1/metadata/tasks       → list_task_definitions
//   GET /api/v1/metadata/workflows   → list_workflow_definitions
//   GET /api/v1/metadata/health      → health_check   ← fully implemented
//
// Usage:
//   MetadataHandler<Protocol>::bind(engine_ctx);
//   // then register static methods as HandlerFn<Protocol> in RouterContext
template <typename Protocol>
class MetadataHandler {
  public:
    // Inject dependencies before the first request arrives.
    static void bind(EngineContext &ctx) noexcept { s_ctx = &ctx; }

    // TODO: IDatabase::query all TaskDef names
    //       encode result as JSON array: [{"name":"...","type":"...","worker_type":"..."}]
    //       cache under key "meta:tasks" with short TTL
    static void list_task_definitions(interfaces::IRequest<Protocol> &req,
                                      interfaces::IResponse<Protocol> &res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

    // TODO: IDatabase::query all WorkflowDef names + latest versions
    //       encode result as JSON array: [{"name":"...","version":1}]
    //       cache under key "meta:workflows" with short TTL
    static void list_workflow_definitions(interfaces::IRequest<Protocol> &req,
                                          interfaces::IResponse<Protocol> &res) noexcept {
        res.set_status(interfaces::Status::NOT_IMPLEMENTED);
    }

    // Implemented: demonstrates the cache-first / DB-probe / bare-response pattern
    // that all other handlers should follow for optional cache+DB usage.
    //
    // Flow:
    //   1. cache present → get "engine:health"
    //      hit  → return cached body immediately
    //      miss → fall through
    //   2. db present → ping (query with op:ping)
    //      → write-through to cache → return ok body
    //   3. neither configured → return bare ok body
    //
    // NOTE: ICache::get and IDatabase::query callbacks are assumed to be invoked
    //       synchronously within the same call frame.  If the backend is async the
    //       caller must block or adapt the pattern accordingly.
    static void health_check(interfaces::IRequest<Protocol> & /*req*/, interfaces::IResponse<Protocol> &res) noexcept {
        static constexpr std::string_view k_cache_key = "engine:health";
        static constexpr std::string_view k_ok = R"({"status":"ok"})";
        static constexpr std::string_view k_bare = R"({"status":"ok","db":false,"cache":false})";

        // Encode a string_view into the response body and set 200 OK.
        auto send = [&](std::string_view body) noexcept {
            std::vector<std::byte> bytes(body.size());
            std::ranges::transform(body, bytes.begin(), [](char c) noexcept { return std::byte(c); });
            res.set_body(std::move(bytes));
            res.set_status(interfaces::Status::OK);
        };

        if (!s_ctx) {
            send(k_bare);
            return;
        }

        // ── Step 1: cache hit ─────────────────────────────────────────────
        if (s_ctx->get_cache()) {
            bool resolved = false;
            s_ctx->get_cache()->get(k_cache_key, [&](std::string_view cached) {
                if (!cached.empty()) {
                    send(cached);
                    resolved = true;
                }
            });
            if (resolved)
                return;
        }

        if (s_ctx->get_db()) {
            s_ctx->get_db()->query(R"({"op":"ping"})", [&](std::string_view /*result*/) {
                if (s_ctx->get_cache()) {
                    s_ctx->get_cache()->set(k_cache_key, k_ok, [](std::string_view) noexcept {});
                }
                send(k_ok);
            });
            return;
        }

        send(k_bare);
    }

  private:
    static inline EngineContext *s_ctx{nullptr};
};

} // namespace engine

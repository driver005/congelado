export module engine:metadata;

import std;
import interfaces;
import model;
import shared;
import serde;
import :context;

export namespace engine {

// Routes:
//   GET /api/v1/metadata/tasks       → list_task_definitions
//   GET /api/v1/metadata/workflows   → list_workflow_definitions
//   GET /api/v1/metadata/health      → health_check
template <typename Protocol>
class MetadataHandler {
  public:
    explicit MetadataHandler(EngineContext &ctx) noexcept : m_ctx{ctx} {}

    void list_task_definitions(interfaces::IRequest<Protocol> &req,
                               interfaces::IResponse<Protocol> &res) noexcept {
        auto accept = req.find_header("accept");

        if (!m_ctx.get().get_db()) {
            reply(res, serde::Ser::serialize_raw(accept, "[]"));
            return;
        }

        m_ctx.get().get_task_def_connector().find_all(
            [&](std::vector<model::TaskDef> tasks) noexcept {
                reply(res, serde::Ser::serialize(accept, tasks));
            });
        m_ctx.get().get_task_def_connector().flush();
    }

    void list_workflow_definitions(interfaces::IRequest<Protocol> &req,
                                   interfaces::IResponse<Protocol> &res) noexcept {
        auto accept = req.find_header("accept");

        if (!m_ctx.get().get_db()) {
            reply(res, serde::Ser::serialize_raw(accept, "[]"));
            return;
        }

        m_ctx.get().get_workflow_def_connector().find_all(
            [&](std::vector<model::WorkflowDef> defs) noexcept {
                reply(res, serde::Ser::serialize(accept, defs));
            });
        m_ctx.get().get_workflow_def_connector().flush();
    }

    void health_check(interfaces::IRequest<Protocol> &req,
                      interfaces::IResponse<Protocol> &res) noexcept {
        static constexpr std::string_view k_cache_key = "engine:health";
        static constexpr std::string_view k_ok = R"({"status":"ok"})";
        static constexpr std::string_view k_bare = R"({"status":"ok","db":false,"cache":false})";

        auto accept = req.find_header("accept");

        if (m_ctx.get().get_cache()) {
            bool done = false;
            m_ctx.get().get_cache()->get(k_cache_key, [&](std::string_view cached) noexcept {
                if (!cached.empty()) {
                    reply(res, serde::Ser::serialize_raw(accept, cached));
                    done = true;
                }
            });
            if (done)
                return;
        }

        if (m_ctx.get().get_db()) {
            m_ctx.get().get_db()->query(
                R"({"op":"ping"})", [&](std::string_view /*result*/) noexcept {
                    if (m_ctx.get().get_cache())
                        m_ctx.get().get_cache()->set(k_cache_key, k_ok,
                                                     [](std::string_view) noexcept {});
                    reply(res, serde::Ser::serialize_raw(accept, k_ok));
                });
            return;
        }

        reply(res, serde::Ser::serialize_raw(accept, k_bare));
    }

  private:
    std::reference_wrapper<EngineContext> m_ctx;

    static void reply(interfaces::IResponse<Protocol> &res, std::vector<std::byte> bytes,
                      interfaces::Status status = interfaces::Status::OK) noexcept {
        res.set_body(std::move(bytes));
        res.set_status(status);
    }
};

} // namespace engine

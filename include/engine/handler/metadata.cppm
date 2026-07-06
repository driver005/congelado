export module engine:metadata;

import std;
import interfaces;
import model;
import shared;
import serde;
import core_logger;
import :context;

export namespace engine {

// Routes:
//   GET /api/v1/metadata/tasks       → list_task_definitions
//   GET /api/v1/metadata/workflows   → list_workflow_definitions
//   GET /api/v1/metadata/health      → health_check
class MetadataHandler {
  public:
    explicit MetadataHandler(EngineContext &ctx) noexcept : m_ctx{ctx} {}

    void list_task_definitions(interfaces::io::IRequest &req,
                               interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");

        m_ctx.get().get_connector().find_all<model::TaskDef>(
            [&](std::vector<model::TaskDef> tasks) noexcept {
                reply(res, serde::Ser::serialize(accept, tasks));
            });
    }

    void list_workflow_definitions(interfaces::io::IRequest &req,
                                   interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");

        m_ctx.get().get_connector().find_all<model::WorkflowDef>(
            [&](std::vector<model::WorkflowDef> defs) noexcept {
                reply(res, serde::Ser::serialize(accept, defs));
            });
    }

    void health_check(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        static constexpr std::string_view CACHE_KEY = "engine:health";
        static constexpr std::string_view OKE = R"({"status":"ok"})";
        static constexpr std::string_view BARE = R"({"status":"ok","db":false,"cache":false})";

        auto accept = req.find_header("accept");

        if (m_ctx.get().get_cache() != nullptr) {
            bool done = false;
            m_ctx.get().get_cache()->get(CACHE_KEY, [&](std::string_view cached) noexcept {
                if (!cached.empty()) {
                    reply(res, serde::Ser::serialize_raw(accept, cached));
                    done = true;
                }
            });
            if (done) {
                return;
            }
        }

        if (m_ctx.get().get_db() != nullptr) {
            m_ctx.get().get_db()->query(
                R"({"op":"ping"})", [&](std::string_view /*result*/) noexcept {
                    if (m_ctx.get().get_cache()) {
                        m_ctx.get().get_cache()->set(CACHE_KEY, OKE,
                                                     [](std::string_view) noexcept {});
                    }
                    reply(res, serde::Ser::serialize_raw(accept, OKE));
                });
            return;
        }

        core::logger::warning("engine", "health: no db or cache");
        reply(res, serde::Ser::serialize_raw(accept, BARE));
    }

  private:
    std::reference_wrapper<EngineContext> m_ctx;

    static void
    reply(interfaces::io::IResponse &res, std::vector<std::byte> bytes,
          interfaces::io::types::Status status = interfaces::io::types::Status::OK) noexcept {
        res.set_body(std::move(bytes));
        res.set_status(status);
    }
};

} // namespace engine

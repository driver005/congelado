export module engine:admin_handler;

import std;
import interfaces;
import model;
import shared;
import serde;
import core_logger;
import :context;
#ifdef CONGELADO_TEST
import io_layer_http2;
import boost.ut;
#endif

export namespace engine {

/// @brief Response body for `GET /api/v1/admin/config` — a snapshot of the engine's currently
/// wired-in backends, not the full congelado.toml (nothing else in this plugin has a handle to
/// that file, only to the already-resolved capability pointers EngineContext holds).
class AdminConfig
{
public:
    AdminConfig() = default;

    AdminConfig(
        bool db_configured, bool lua_bridge_configured, std::uint32_t sweep_interval_seconds
    ) :
        m_db_configured{db_configured},
        m_lua_bridge_configured{lua_bridge_configured},
        m_sweep_interval_seconds{sweep_interval_seconds}
    {
    }

    void set_db_configured(bool value) noexcept
    {
        m_db_configured = value;
    }

    [[nodiscard]] bool get_db_configured() const noexcept
    {
        return m_db_configured;
    }

    void set_lua_bridge_configured(bool value) noexcept
    {
        m_lua_bridge_configured = value;
    }

    [[nodiscard]] bool get_lua_bridge_configured() const noexcept
    {
        return m_lua_bridge_configured;
    }

    void set_sweep_interval_seconds(std::uint32_t value) noexcept
    {
        m_sweep_interval_seconds = value;
    }

    [[nodiscard]] std::uint32_t get_sweep_interval_seconds() const noexcept
    {
        return m_sweep_interval_seconds;
    }

private:
    bool m_db_configured{false};
    bool m_lua_bridge_configured{false};
    std::uint32_t m_sweep_interval_seconds{0};
};

// Routes:
//   POST /api/v1/admin/consistency/:exec_id → consistency (self-healing reconcile)
//   GET  /api/v1/admin/config               → config
class AdminHandler
{
public:
    explicit AdminHandler(EngineContext& ctx) noexcept :
        m_ctx{ctx}
    {
    }

    /**
     * @brief Handles `POST /api/v1/admin/consistency/:exec_id` — self-healing: re-derives what
     * should be scheduled next by re-running the DAG-advance logic, same as Orchestrator::
     * reconcile() already does for a stuck execution (a worker crashed mid-write, a sweep tick
     * was missed, etc). Thin wrapper, all the real logic already lives in Orchestrator.
     * @param req the inbound request; path supplies the exec_id.
     * @param res the response — 200 on success, 404 if the execution or its def wasn't found.
     */
    void consistency(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    )
    {
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto exec_id = std::string{target.substr(target.rfind('/') + 1)};
        m_ctx.get().get_workflow_orchestrator()->reconcile(
            exec_id, [&res, accept, send = std::move(send)](bool oke) {
                if (!oke) {
                    reply(
                        res, serde::Ser::serialize_error(accept, "not found"),
                        interfaces::io::types::Status::NOT_FOUND
                    );
                    send();
                    return;
                }
                res.set_status(interfaces::io::types::Status::OK);
                send();
            }
        );
    }

    /**
     * @brief Handles `GET /api/v1/admin/config` — reports which backends this engine instance
     * currently has wired in.
     * @param req the inbound request; only its Accept header gets read here.
     * @param res the response this writes the serialized AdminConfig into.
     */
    void config(
        interfaces::io::IRequest& req, interfaces::io::IResponse& res, std::function<void()> send
    ) noexcept
    {
        auto accept = req.find_header("accept");
        AdminConfig cfg{
            m_ctx.get().get_db() != nullptr, m_ctx.get().get_lua_bridge() != nullptr, 5
        };
        reply(res, serde::Ser::serialize(accept, cfg));
        send();
    }

private:
    std::reference_wrapper<EngineContext> m_ctx;

    static void reply(
        interfaces::io::IResponse& res,
        std::vector<std::byte> bytes,
        interfaces::io::types::Status status = interfaces::io::types::Status::OK
    ) noexcept
    {
        res.set_body(std::move(bytes));
        res.set_status(status);
    }
};

} // namespace engine

template<>
struct serde::Serializable<engine::AdminConfig>
{
    static constexpr auto fields()
    {
        return std::tuple{
            serde::FieldDesc<
                "db_configured", &engine::AdminConfig::get_db_configured,
                &engine::AdminConfig::set_db_configured>{},
            serde::FieldDesc<
                "lua_bridge_configured", &engine::AdminConfig::get_lua_bridge_configured,
                &engine::AdminConfig::set_lua_bridge_configured>{},
            serde::FieldDesc<
                "sweep_interval_seconds", &engine::AdminConfig::get_sweep_interval_seconds,
                &engine::AdminConfig::set_sweep_interval_seconds>{},
        };
    }
};

#ifdef CONGELADO_TEST
namespace engine::admin_handler_tests {
using namespace boost::ut;

// Minimal IWorkflowOrchestrator fake — only reconcile()'s outcome is configurable, every other
// pure virtual is a harmless no-op/default just to make this class instantiable.
class FakeWorkflowOrchestrator final : public interfaces::IWorkflowOrchestrator
{
public:
    explicit FakeWorkflowOrchestrator(bool reconcile_result) noexcept :
        m_reconcile_result{reconcile_result}
    {
    }

    [[nodiscard]] std::string_view backend_name() const noexcept override
    {
        return "fake";
    }

    void start_workflow(
        std::string_view,
        const std::unordered_map<std::string, std::string>&,
        std::move_only_function<void(std::optional<std::string>)> callback
    ) override
    {
        callback(std::nullopt);
    }

    void on_task_terminal(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void on_execution_terminal(
        std::string_view, std::move_only_function<void(bool)> callback
    ) override
    {
        callback(true);
    }

    void pause(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void resume(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void retry(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void restart(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void terminate(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(true);
    }

    void reconcile(std::string_view, std::move_only_function<void(bool)> callback) override
    {
        callback(m_reconcile_result);
    }

    void rerun(
        std::string_view,
        std::string_view,
        const interfaces::Value&,
        std::move_only_function<void(bool)> callback
    ) override
    {
        callback(true);
    }

    void signal(
        std::string_view,
        std::string_view,
        std::optional<std::string_view>,
        std::move_only_function<void(bool)> callback
    ) override
    {
        callback(true);
    }

    void complete_task(
        std::string_view,
        std::string_view,
        bool,
        const std::unordered_map<std::string, std::string>&,
        std::move_only_function<void(bool)> callback
    ) override
    {
        callback(true);
    }

    void start_server() override {}

    void shutdown_all() override {}

private:
    bool m_reconcile_result;
};

suite<"AdminConfig"> admin_config_suite = [] {
    "default-constructs with false/false/0"_test = [] {
        engine::AdminConfig cfg;
        expect(!cfg.get_db_configured());
        expect(!cfg.get_lua_bridge_configured());
        expect(cfg.get_sweep_interval_seconds() == 0);
    };

    "value ctor sets every field"_test = [] {
        engine::AdminConfig cfg{true, true, 5};
        expect(cfg.get_db_configured());
        expect(cfg.get_lua_bridge_configured());
        expect(cfg.get_sweep_interval_seconds() == 5);
    };

    "set_db_configured/get_db_configured round-trip"_test = [] {
        engine::AdminConfig cfg;
        cfg.set_db_configured(true);
        expect(cfg.get_db_configured());
        cfg.set_db_configured(false);
        expect(!cfg.get_db_configured());
    };

    "set_lua_bridge_configured/get_lua_bridge_configured round-trip"_test = [] {
        engine::AdminConfig cfg;
        cfg.set_lua_bridge_configured(true);
        expect(cfg.get_lua_bridge_configured());
        cfg.set_lua_bridge_configured(false);
        expect(!cfg.get_lua_bridge_configured());
    };

    "set_sweep_interval_seconds/get_sweep_interval_seconds round-trip"_test = [] {
        engine::AdminConfig cfg;
        cfg.set_sweep_interval_seconds(42);
        expect(cfg.get_sweep_interval_seconds() == 42);
    };
};

// SECURITY: pins routes.cppm's "no auth middleware anywhere" finding for this handler
// specifically — every case below drives consistency()/config() directly with no auth setup of
// any kind (no token, no header, nothing), and each still replies normally (200/404), never
// 401/403. That's the confirmation: there's no auth gate to trip.
suite<"AdminHandler"> admin_handler_suite = [] {
    "config reports db_configured/lua_bridge_configured false on an unwired context"_test = [] {
        engine::EngineContext ctx;
        engine::AdminHandler handler{ctx};
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_header("accept", "application/json");
        bool sent = false;

        handler.config(req, res, [&sent] {
            sent = true;
        });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::OK);
    };

    "consistency replies 200 when the workflow orchestrator reconciles successfully"_test = [] {
        engine::EngineContext ctx;
        FakeWorkflowOrchestrator orchestrator{true};
        ctx.set_workflow_orchestrator(&orchestrator);
        engine::AdminHandler handler{ctx};
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_header(interfaces::io::types::Token::PATH, "/api/v1/admin/consistency/exec-123");
        bool sent = false;

        handler.consistency(req, res, [&sent] {
            sent = true;
        });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::OK);
    };

    "consistency replies 404 when the workflow orchestrator can't reconcile"_test = [] {
        engine::EngineContext ctx;
        FakeWorkflowOrchestrator orchestrator{false};
        ctx.set_workflow_orchestrator(&orchestrator);
        engine::AdminHandler handler{ctx};
        io::layer::http2::HttpRequest req{1};
        io::layer::http2::HttpResponse res{1};
        req.set_header(interfaces::io::types::Token::PATH, "/api/v1/admin/consistency/exec-404");
        bool sent = false;

        handler.consistency(req, res, [&sent] {
            sent = true;
        });

        expect(sent);
        expect(res.get_status() == interfaces::io::types::Status::NOT_FOUND);
    };
};

} // namespace engine::admin_handler_tests
#endif

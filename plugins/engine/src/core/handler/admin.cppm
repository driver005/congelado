export module engine:admin_handler;

import std;
import interfaces;
import model;
import shared;
import serde;
import core_logger;
import :context;
import :orchestrator;

export namespace engine {

/// @brief Response body for `GET /api/v1/admin/config` — a snapshot of the engine's currently
/// wired-in backends, not the full congelado.toml (nothing else in this plugin has a handle to
/// that file, only to the already-resolved capability pointers EngineContext holds).
class AdminConfig {
  public:
    AdminConfig() = default;
    AdminConfig(bool db_configured, bool lua_bridge_configured, std::uint32_t sweep_interval_seconds)
        : m_db_configured{db_configured}, m_lua_bridge_configured{lua_bridge_configured},
          m_sweep_interval_seconds{sweep_interval_seconds} {}

    void set_db_configured(bool value) noexcept { m_db_configured = value; }
    [[nodiscard]] bool get_db_configured() const noexcept { return m_db_configured; }
    void set_lua_bridge_configured(bool value) noexcept { m_lua_bridge_configured = value; }
    [[nodiscard]] bool get_lua_bridge_configured() const noexcept { return m_lua_bridge_configured; }
    void set_sweep_interval_seconds(std::uint32_t value) noexcept { m_sweep_interval_seconds = value; }
    [[nodiscard]] std::uint32_t get_sweep_interval_seconds() const noexcept {
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
class AdminHandler {
  public:
    explicit AdminHandler(EngineContext &ctx) noexcept : m_ctx{ctx} {}

    /**
     * @brief Handles `POST /api/v1/admin/consistency/:exec_id` — self-healing: re-derives what
     * should be scheduled next by re-running the DAG-advance logic, same as Orchestrator::
     * reconcile() already does for a stuck execution (a worker crashed mid-write, a sweep tick
     * was missed, etc). Thin wrapper, all the real logic already lives in Orchestrator.
     * @param req the inbound request; path supplies the exec_id.
     * @param res the response — 200 on success, 404 if the execution or its def wasn't found.
     */
    void consistency(interfaces::io::IRequest &req, interfaces::io::IResponse &res) {
        auto accept = req.find_header("accept");
        auto target = req.get_path();
        auto exec_id = std::string{target.substr(target.rfind('/') + 1)};
        Orchestrator{m_ctx.get()}.reconcile(exec_id, [&res, accept](bool oke) {
            if (!oke) {
                reply(res, serde::Ser::serialize_error(accept, "not found"),
                      interfaces::io::types::Status::NOT_FOUND);
                return;
            }
            res.set_status(interfaces::io::types::Status::OK);
        });
    }

    /**
     * @brief Handles `GET /api/v1/admin/config` — reports which backends this engine instance
     * currently has wired in.
     * @param req the inbound request; only its Accept header gets read here.
     * @param res the response this writes the serialized AdminConfig into.
     */
    void config(interfaces::io::IRequest &req, interfaces::io::IResponse &res) noexcept {
        auto accept = req.find_header("accept");
        AdminConfig cfg{m_ctx.get().get_db() != nullptr, m_ctx.get().get_lua_bridge() != nullptr, 5};
        reply(res, serde::Ser::serialize(accept, cfg));
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

template <>
struct serde::Serializable<engine::AdminConfig> {
    static constexpr auto fields() {
        return std::tuple{
            serde::FieldDesc<"db_configured", &engine::AdminConfig::get_db_configured,
                             &engine::AdminConfig::set_db_configured>{},
            serde::FieldDesc<"lua_bridge_configured", &engine::AdminConfig::get_lua_bridge_configured,
                             &engine::AdminConfig::set_lua_bridge_configured>{},
            serde::FieldDesc<"sweep_interval_seconds", &engine::AdminConfig::get_sweep_interval_seconds,
                             &engine::AdminConfig::set_sweep_interval_seconds>{},
        };
    }
};

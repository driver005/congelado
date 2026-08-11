export module congelado_heart:context;

import std;
import interfaces;
import io_shared;
import core_router;
import core_contract;
import io_base_leverage;
import core_logger;
import core_events;
import core_otel;
import serde;
import connector;

export namespace congelado::heart {

class AppContext {
  public:
    /**
     * @brief Spins up the app-wide context — router, contract group, and leverager all default
     * to empty, and a contract thread pool gets emplaced against the contract group right away
     * so there's motion the moment a plugin needs it. The pool's worker-thread count comes from
     * the top-level `threads` config key (see core::config::Config::get_threads); defaults to 1.
     * @param thread_count number of worker threads to run in the contract thread pool.
     */
    explicit AppContext(std::size_t thread_count = std::thread::hardware_concurrency()) {
        m_thread_pool.emplace(m_contract_group, thread_count);
        m_connector = std::make_unique<connector::Connector>();
    }

    /**
     * @brief Stops and joins the contract thread pool's worker thread(s), ahead of (and
     * separate from) this `AppContext`'s own destruction.
     * @warning Call this before tearing down any plugin whose state a queued contract job might
     * still touch (e.g. via `core::plugin::SharedLibrary::close_all()`) — a worker thread can be
     * mid-job, reading plugin-owned objects (a protocol plugin's socket/`Endpoint`, say) at the
     * exact moment `on_unload()` frees them, which is a real use-after-free, not a hypothetical
     * one. No-op if already stopped.
     */
    void stop_thread_pool() noexcept { m_thread_pool.reset(); }

    /// @brief Gets the shared router context handed to every loading plugin. @return a pointer
    /// to the owned `RouterContext`, never null.
    [[nodiscard]] core::router::RouterContext<> *get_router() noexcept { return &m_router; }

    /// @brief Gets the contract group backing the thread pool. @return a reference to the owned
    /// `ContractGroup`.
    [[nodiscard]] core::contract::ContractGroup<> &get_contract_group() noexcept {
        return m_contract_group;
    }

    /// @brief Gets the leverager shared across plugins for I/O leverage work. @return a
    /// reference to the owned `Leverager`.
    [[nodiscard]] io::base::leverage::Leverager<io::base::leverage::Context> &
    get_leverager() noexcept {
        return m_leverager;
    }

    /// @brief Gets the process's one logger registry — `ServerRunner::run()` points the ambient
    /// `core::logger::*` facade at this via `set_active()` right after construction.
    /// @return a reference to the owned `LoggerRegistry`.
    [[nodiscard]] core::logger::LoggerRegistry &get_logger_registry() noexcept {
        return m_logger_registry;
    }

    /// @brief Gets the process's one event-bus registry — optional, unlike the logger: no
    /// event-sink plugin loading it just means `core::events::publish(...)` degrades to a debug
    /// log line. `ServerRunner::run()` points the ambient `core::events::publish(...)` facade at
    /// this via `set_active()` right after construction.
    /// @return a reference to the owned `EventBusRegistry`.
    [[nodiscard]] core::events::EventBusRegistry &get_event_bus_registry() noexcept {
        return m_event_bus_registry;
    }

    /// @brief Gets the process's one serde format registry — `ServerRunner::run()` points the
    /// ambient `serde::Ser` facade at this via `set_active()` right after construction.
    /// @return a reference to the owned `SerdeFormatRegistry`.
    [[nodiscard]] serde::SerdeFormatRegistry &get_serde_format_registry() noexcept {
        return m_serde_format_registry;
    }

    /// @brief Gets the process's one tracer registry — optional, unlike the logger: no plugin
    /// loading it just means spans silently no-op. `ServerRunner::run()` points the ambient
    /// `core::otel::start_span(...)` facade at this via `set_active()` right after construction.
    /// @return a reference to the owned `TracerRegistry`.
    [[nodiscard]] core::otel::TracerRegistry &get_tracer_registry() noexcept {
        return m_tracer_registry;
    }

    /// @brief Gets the process's one meter registry — same optional-by-default deal as the
    /// tracer registry.
    /// @return a reference to the owned `MeterRegistry`.
    [[nodiscard]] core::otel::MeterRegistry &get_meter_registry() noexcept {
        return m_meter_registry;
    }

    /// @brief Gets the process's one OTel log-record registry — the export target
    /// `OtelLogBridge` forwards into once registered as a `core::logger` backend.
    /// @return a reference to the owned `LogRecordRegistry`.
    [[nodiscard]] core::otel::LogRecordRegistry &get_log_record_registry() noexcept {
        return m_log_record_registry;
    }

    /// @brief Gets the shared connector this app context owns and registers with the contract
    /// group. @return a pointer to the owned `connector::Connector`, never null.
    [[nodiscard]] connector::Connector *get_connector() noexcept { return m_connector.get(); }

    /// @brief Access to the contract registry for registering/releasing heart-owned contracts.
    [[nodiscard]] core::contract::ContractRegistry &get_contract_registry() noexcept {
        return m_contract_registry;
    }

    /**
     * @brief Clean shutdown order: release all registered contracts first, then stop and join the
     * contract thread pool.
     */
    void stop() noexcept {
        if (m_connector) {
            m_connector->set_wake({});
        }
        m_contract_registry.release_all();
        stop_thread_pool();
    }

  private:
    // Declaration order is destruction order (reverse) — the registries below must outlive
    // m_contract_group/m_leverager/m_thread_pool, since ~ContractThreadPool() logs while
    // joining its worker threads. Keep every ambient-facade registry ahead of them here, or
    // that log call reads through an already-destroyed registry.
    core::router::RouterContext<> m_router;
    core::logger::LoggerRegistry m_logger_registry;
    core::events::EventBusRegistry m_event_bus_registry;
    serde::SerdeFormatRegistry m_serde_format_registry;
    core::otel::TracerRegistry m_tracer_registry;
    core::otel::MeterRegistry m_meter_registry;
    core::otel::LogRecordRegistry m_log_record_registry;
    std::unique_ptr<connector::Connector> m_connector;
    core::contract::ContractRegistry m_contract_registry;
    core::contract::ContractGroup<> m_contract_group;
    io::base::leverage::Leverager<io::base::leverage::Context> m_leverager;
    std::optional<core::contract::ContractThreadPool<>> m_thread_pool;
};

} // namespace congelado::heart

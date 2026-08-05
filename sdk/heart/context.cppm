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

export namespace congelado::heart {

class AppContext {
  public:
    /**
     * @brief Spins up the app-wide context — router, contract group, and leverager all default
     * to empty, and a single-thread contract thread pool gets emplaced against the contract
     * group right away so there's motion the moment a plugin needs it.
     */
    AppContext() { m_thread_pool.emplace(m_contract_group, 1); }

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

  private:
    core::contract::ContractGroup<> m_contract_group;
    io::base::leverage::Leverager<io::base::leverage::Context> m_leverager;
    std::optional<core::contract::ContractThreadPool<>> m_thread_pool;

    core::router::RouterContext<> m_router;
    core::logger::LoggerRegistry m_logger_registry;
    core::events::EventBusRegistry m_event_bus_registry;
    serde::SerdeFormatRegistry m_serde_format_registry;
    core::otel::TracerRegistry m_tracer_registry;
    core::otel::MeterRegistry m_meter_registry;
    core::otel::LogRecordRegistry m_log_record_registry;
};

} // namespace congelado::heart

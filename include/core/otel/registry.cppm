export module core_otel:registry;

import std;
import interfaces;

export namespace core::otel {

/**
 * @brief Holds every registered tracer provider for one process — mirrors
 * `core::logger::LoggerRegistry` exactly: instance-owned (not a static singleton), fan-out
 * (every registered provider receives every span), `set_active()` points the ambient
 * `core::otel::start_span(...)`-style facade at it. Only `s_active` — a single pointer, not the
 * registry data itself — is process-global.
 */
class TracerRegistry {
  public:
    /**
     * @brief Registers a tracer provider so it starts receiving every fanned-out span.
     * @note No-op if `provider` is null. Once registered there's no unregister.
     * @param provider the provider instance to add.
     */
    void add_provider(std::shared_ptr<interfaces::ITracerProvider> provider) {
        if (provider) {
            m_providers.push_back(std::move(provider));
        }
    }

    /**
     * @brief Checks whether the registry currently holds any provider.
     * @return true if at least one provider is registered.
     */
    [[nodiscard]] bool has_provider() const noexcept { return !m_providers.empty(); }

    /**
     * @brief Gets every provider currently registered, in registration order.
     * @return the full list of registered tracer providers.
     */
    [[nodiscard]] const std::vector<std::shared_ptr<interfaces::ITracerProvider>> &
    get_providers() const noexcept {
        return m_providers;
    }

    /**
     * @brief Points the ambient tracing facade at this instance.
     * @param registry the instance to make active, or `nullptr` to clear it.
     */
    static void set_active(TracerRegistry *registry) noexcept { s_active = registry; }

    /**
     * @brief Gets the currently active registry, if one was set.
     * @return the active `TracerRegistry`, or `nullptr` if `set_active()` was never called.
     */
    [[nodiscard]] static TracerRegistry *get_active() noexcept { return s_active; }

  private:
    std::vector<std::shared_ptr<interfaces::ITracerProvider>> m_providers;
    static inline TracerRegistry *s_active{nullptr};
};

/**
 * @brief Holds every registered meter provider for one process — same fan-out/instance-owned/
 * ambient-pointer shape as `TracerRegistry`/`LoggerRegistry`.
 */
class MeterRegistry {
  public:
    /**
     * @brief Registers a meter provider so it starts receiving every fanned-out metric point.
     * @note No-op if `provider` is null. Once registered there's no unregister.
     * @param provider the provider instance to add.
     */
    void add_provider(std::shared_ptr<interfaces::IMeterProvider> provider) {
        if (provider) {
            m_providers.push_back(std::move(provider));
        }
    }

    /**
     * @brief Checks whether the registry currently holds any provider.
     * @return true if at least one provider is registered.
     */
    [[nodiscard]] bool has_provider() const noexcept { return !m_providers.empty(); }

    /**
     * @brief Gets every provider currently registered, in registration order.
     * @return the full list of registered meter providers.
     */
    [[nodiscard]] const std::vector<std::shared_ptr<interfaces::IMeterProvider>> &
    get_providers() const noexcept {
        return m_providers;
    }

    /**
     * @brief Points the ambient metrics facade at this instance.
     * @param registry the instance to make active, or `nullptr` to clear it.
     */
    static void set_active(MeterRegistry *registry) noexcept { s_active = registry; }

    /**
     * @brief Gets the currently active registry, if one was set.
     * @return the active `MeterRegistry`, or `nullptr` if `set_active()` was never called.
     */
    [[nodiscard]] static MeterRegistry *get_active() noexcept { return s_active; }

  private:
    std::vector<std::shared_ptr<interfaces::IMeterProvider>> m_providers;
    static inline MeterRegistry *s_active{nullptr};
};

/**
 * @brief Holds every registered log-record provider for one process — the export target
 * `OtelLogBridge` (an `interfaces::ILogger` registered into the *existing*
 * `core::logger::LoggerRegistry`) forwards every log record to. Same fan-out/instance-owned/
 * ambient-pointer shape as the other two registries here.
 */
class LogRecordRegistry {
  public:
    /**
     * @brief Registers a log-record provider so it starts receiving every fanned-out log record.
     * @note No-op if `provider` is null. Once registered there's no unregister.
     * @param provider the provider instance to add.
     */
    void add_provider(std::shared_ptr<interfaces::ILogRecordProvider> provider) {
        if (provider) {
            m_providers.push_back(std::move(provider));
        }
    }

    /**
     * @brief Checks whether the registry currently holds any provider.
     * @return true if at least one provider is registered.
     */
    [[nodiscard]] bool has_provider() const noexcept { return !m_providers.empty(); }

    /**
     * @brief Gets every provider currently registered, in registration order.
     * @return the full list of registered log-record providers.
     */
    [[nodiscard]] const std::vector<std::shared_ptr<interfaces::ILogRecordProvider>> &
    get_providers() const noexcept {
        return m_providers;
    }

    /**
     * @brief Points the ambient `OtelLogBridge` at this instance.
     * @param registry the instance to make active, or `nullptr` to clear it.
     */
    static void set_active(LogRecordRegistry *registry) noexcept { s_active = registry; }

    /**
     * @brief Gets the currently active registry, if one was set.
     * @return the active `LogRecordRegistry`, or `nullptr` if `set_active()` was never called.
     */
    [[nodiscard]] static LogRecordRegistry *get_active() noexcept { return s_active; }

    /**
     * @brief Drops every registered provider.
     * @warning Call this before tearing down the plugins that own those providers (e.g. via
     * `core::plugin::SharedLibrary::close_all()`) — `OtelLogBridge::emit()` fans every
     * `core::logger::*` call out to each provider here, and a plugin's own shutdown path can
     * itself log (OTel's SDK does, from inside its own `Shutdown()`). Without clearing first,
     * that reenters straight back into the same provider mid-teardown — confirmed live as a
     * segfault. `has_provider()` returns `false` immediately after this, so `emit()` no-ops
     * instead of touching a soon-to-be-dangling provider pointer.
     */
    void clear() noexcept { m_providers.clear(); }

  private:
    std::vector<std::shared_ptr<interfaces::ILogRecordProvider>> m_providers;
    static inline LogRecordRegistry *s_active{nullptr};
};

} // namespace core::otel

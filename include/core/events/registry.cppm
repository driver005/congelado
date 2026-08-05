export module core_events:registry;

import std;
import interfaces;

export namespace core::events {

/**
 * @brief Holds every registered event sink for one process. Instance-owned (not a static
 * singleton) — same shape as `core::logger::LoggerRegistry`: exactly one lives inside
 * `congelado::heart::AppContext`, and `set_active()` points the ambient `core::events::publish()`
 * free-function facade at it. Only `s_active` — a single pointer, not the sink data itself — is
 * process-global. Multiple sinks can be registered at once and all receive every published
 * event, same fan-out story as `LoggerRegistry`.
 */
class EventBusRegistry {
  public:
    /**
     * @brief Registers a sink so it starts receiving every published event.
     * @note No-op if `sink` is null — silently dropped, no error, no throw. Once registered
     * there's no unregister — it's riding with this instance for good.
     * @param sink the sink instance to add to the registry.
     */
    void add_sink(std::shared_ptr<interfaces::IEventSink> sink) {
        if (sink) {
            m_sinks.push_back(std::move(sink));
        }
    }

    /**
     * @brief Checks whether the registry currently holds any sink.
     * @return true if at least one sink is registered, false if it's still empty.
     */
    [[nodiscard]] bool has_sink() const noexcept { return !m_sinks.empty(); }

    /**
     * @brief Gets every sink currently registered, in registration order.
     * @return the full list of registered sinks.
     */
    [[nodiscard]] const std::vector<std::shared_ptr<interfaces::IEventSink>> &
    get_sinks() const noexcept {
        return m_sinks;
    }

    /**
     * @brief Points the ambient publish facade at this instance — call once, right after
     * constructing the process's one `EventBusRegistry`, before any `core::events::publish()`
     * call.
     * @param registry the instance to make active, or `nullptr` to clear it.
     */
    static void set_active(EventBusRegistry *registry) noexcept { s_active = registry; }

    /**
     * @brief Gets the currently active registry, if one was set.
     * @return the active `EventBusRegistry`, or `nullptr` if `set_active()` was never called.
     */
    [[nodiscard]] static EventBusRegistry *get_active() noexcept { return s_active; }

  private:
    std::vector<std::shared_ptr<interfaces::IEventSink>> m_sinks;
    static inline EventBusRegistry *s_active{nullptr};
};

} // namespace core::events

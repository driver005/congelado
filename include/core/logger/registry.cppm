export module core_logger:registry;

import std;
import interfaces;

export namespace core::logger {

/**
 * @brief Holds every registered logger for one process. Instance-owned (not a static
 * singleton) — exactly one lives inside `congelado::heart::AppContext` (the engine) or a local
 * variable in the worker's `main()`, and `set_active()` points the ambient
 * `core::logger::error(...)`-style free-function facade (~417 call sites across the codebase,
 * deliberately kept untouched) at it. Only `s_active` — a single pointer, not the logger data
 * itself — is process-global.
 */
class LoggerRegistry {
  public:
    // Appends a logger. No-op if null. Multiple loggers all receive every message.
    /**
     * @brief Registers a logger so it starts catching every fanned-out log call.
     * @note No-op if `logger` is null — silently dropped, no error, no throw. Once
     * registered there's no unregister — it's riding with this instance for good.
     * @param logger the logger instance to add to the registry.
     */
    void add_logger(std::shared_ptr<interfaces::ILogger> logger) {
        if (logger) {
            m_loggers.push_back(std::move(logger));
        }
    }

    /**
     * @brief Checks whether the registry currently holds any logger.
     * @return true if at least one logger is registered, false if it's still empty.
     */
    [[nodiscard]] bool has_logger() const noexcept { return !m_loggers.empty(); }

    /**
     * @brief Gets every logger currently registered, in registration order.
     * @return the full list of registered loggers.
     */
    [[nodiscard]] const std::vector<std::shared_ptr<interfaces::ILogger>> &get_loggers() const noexcept {
        return m_loggers;
    }

    /**
     * @brief Points the ambient logging facade at this instance — call once, right after
     * constructing the process's one `LoggerRegistry`, before any `core::logger::*` call.
     * @param registry the instance to make active, or `nullptr` to clear it.
     */
    static void set_active(LoggerRegistry *registry) noexcept { s_active = registry; }

    /**
     * @brief Gets the currently active registry, if one was set.
     * @return the active `LoggerRegistry`, or `nullptr` if `set_active()` was never called.
     */
    [[nodiscard]] static LoggerRegistry *get_active() noexcept { return s_active; }

  private:
    std::vector<std::shared_ptr<interfaces::ILogger>> m_loggers;
    static inline LoggerRegistry *s_active{nullptr};
};

} // namespace core::logger

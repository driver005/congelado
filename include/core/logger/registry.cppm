export module core_logger:registry;

import std;
import interfaces;

export namespace core::logger {

class LoggerRegistry {
    static inline std::vector<std::shared_ptr<interfaces::ILogger>> loggers;

  public:
    // Appends a logger. No-op if null. Multiple loggers all receive every message.
    /**
     * @brief Registers a logger so it starts catching every fanned-out log call.
     * @note No-op if `logger` is null — silently dropped, no error, no throw. Once
     * registered there's no unregister — it's riding with the process for good.
     * @param logger the logger instance to add to the registry.
     */
    static void register_logger(std::shared_ptr<interfaces::ILogger> logger) {
        if (logger) {
            loggers.push_back(std::move(logger));
        }
    }

    /**
     * @brief Checks whether the registry currently holds any logger.
     * @return true if at least one logger is registered, false if it's still empty.
     */
    [[nodiscard]] static bool has_logger() noexcept { return !loggers.empty(); }

    /**
     * @brief Gets every logger currently registered, in registration order.
     * @return the full list of registered loggers.
     */
    [[nodiscard]] static const std::vector<std::shared_ptr<interfaces::ILogger>> &all() noexcept {
        return loggers;
    }
};

} // namespace core::logger

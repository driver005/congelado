export module core_logger:registry;

import std;
import interfaces;

export namespace core::logger {

class LoggerRegistry {
    static inline std::vector<std::shared_ptr<interfaces::ILogger>> loggers;

  public:
    // Appends a logger. No-op if null. Multiple loggers all receive every message.
    static void register_logger(std::shared_ptr<interfaces::ILogger> logger) {
        if (logger) loggers.push_back(std::move(logger));
    }

    [[nodiscard]] static bool has_logger() noexcept { return !loggers.empty(); }

    [[nodiscard]] static const std::vector<std::shared_ptr<interfaces::ILogger>> &all() noexcept {
        return loggers;
    }
};

} // namespace core::logger

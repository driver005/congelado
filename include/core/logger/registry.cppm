export module core_logger:registry;

import std;
import interfaces;

export namespace core::logger {

class LoggerRegistry {
  private:
    static inline std::shared_ptr<interfaces::ILogger> current_logger = nullptr;

  public:
    // Registers a plugin logger and returns its Settings string.
    static std::string register_logger(std::shared_ptr<interfaces::ILogger> logger) {
        current_logger = std::move(logger);
        if (current_logger) {
            return current_logger->initialize();
        }
        return "Settings: None";
    }

    static interfaces::ILogger *get() { return current_logger.get(); }
};

} // namespace core::logger

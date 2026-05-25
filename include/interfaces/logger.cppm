export module interfaces:logger;

import std;

export namespace interfaces {

enum class LogLevel { Debug, Info, Warning, Error, Fatal };

constexpr std::string_view to_string(LogLevel level) {
    switch (level) {
    case LogLevel::Debug:
        return "DEBUG";
    case LogLevel::Info:
        return "INFO";
    case LogLevel::Warning:
        return "WARNING";
    case LogLevel::Error:
        return "ERROR";
    case LogLevel::Fatal:
        return "FATAL";
    }
    return "UNKNOWN";
}

class ILogger {
  public:
    virtual ~ILogger() = default;

    [[nodiscard]] virtual std::string_view name() const noexcept = 0;

    // The actual logging endpoint. No templates here to keep ABI stable across plugins.
    virtual void write(LogLevel level, std::string_view message) noexcept = 0;

    virtual void error(std::string_view message) noexcept = 0;
};

} // namespace interfaces

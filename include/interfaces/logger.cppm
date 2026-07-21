export module interfaces:logger;

import std;

export namespace interfaces {

enum class LogLevel : std::uint8_t { DEBUG, INFO, IMPORTANT, WARNING, ERROR, FATAL };

constexpr std::string_view to_string(LogLevel level) {
    switch (level) {
    case LogLevel::DEBUG:
        return "DEBUG";
    case LogLevel::INFO:
        return "INFO";
    case LogLevel::IMPORTANT:
        return "IMPORTANT";
    case LogLevel::WARNING:
        return "WARNING";
    case LogLevel::ERROR:
        return "ERROR";
    case LogLevel::FATAL:
        return "FATAL";
    }
    // Switch is exhaustive over every LogLevel value, so this only ever fires if something cast
    // in a bogus/out-of-range level from outside the enum — defensive fallback, not dead code.
    return "UNKNOWN";
}

class ILogger {
  public:
    /// @brief Default ctor — kept explicit since declaring the copy/move members below would
    /// otherwise suppress it, and derived loggers (e.g. Plugin, LoggerAdapter) rely on default
    /// constructing this base.
    ILogger() = default;

    /**
     * @brief Virtual dtor, default's fine — loggers clean up fine through the base pointer,
     * nothing else needed, straight bet.
     */
    virtual ~ILogger() = default;

    /**
     * @brief Copy ctor, defaulted — no data members of its own, so member-wise copy is trivially
     * correct.
     */
    ILogger(const ILogger &) = default;
    /**
     * @brief Copy assignment, defaulted alongside the copy ctor for the same reason.
     */
    ILogger &operator=(const ILogger &) = default;
    /**
     * @brief Move ctor, defaulted — same story, nothing owned that needs special handling.
     */
    ILogger(ILogger &&) = default;
    /**
     * @brief Move assignment, defaulted to round out the set.
     */
    ILogger &operator=(ILogger &&) = default;

    /**
     * @brief Tells you which logger you're actually holding onto (console, file, whatever sink
     * got wired in) — lowkey just its name tag.
     * @return the logger's name.
     */
    [[nodiscard]] virtual std::string_view get_name() const noexcept = 0;

    // The actual logging endpoint. No templates here to keep ABI stable across plugins.
    /**
     * @brief Ships a log line at the given severity — this is the real write path, kept
     * non-templated on purpose so the ABI stays stable across plugin boundaries. Templates and
     * stable ABIs don't mix, so this stays boring and concrete on purpose.
     * @param level how serious this line is, debug all the way up through fatal.
     * @param message the text getting logged.
     */
    virtual void write(LogLevel level, std::string_view message) noexcept = 0;

    /**
     * @brief Shortcut for logging straight at Error severity without spelling out the level
     * every single time — for when something's already an L and you know it.
     * @param message the error text getting logged.
     */
    virtual void error(std::string_view message) noexcept = 0;
};

} // namespace interfaces

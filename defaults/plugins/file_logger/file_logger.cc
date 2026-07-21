#define CONGELADO_GUEST
import congelado_plugin;
#include <congelado/plugin.h>
import std;

namespace {

constexpr int parse_level(std::string_view level) noexcept {
    if (level == "debug") {
        return 0;
    }
    if (level == "info") {
        return 1;
    }
    if (level == "important") {
        return 2;
    }
    if (level == "warning") {
        return 3;
    }
    if (level == "error") {
        return 4;
    }
    if (level == "fatal") {
        return 5;
    }
    return 1;
}

constexpr std::string_view level_str(int level) noexcept {
    switch (level) {
    case 0:
        return "DEBUG";
    case 1:
        return "INFO";
    case 2:
        return "IMPORTANT";
    case 3:
        return "WARNING";
    case 4:
        return "ERROR";
    case 5:
        return "FATAL";
    default:
        return "UNKNOWN";
    }
}

class FileLogger final : public congelado::Plugin {
  public:
    /**
     * @brief Plugin name reported to the host.
     * @return `"FileLogger"`.
     */
    [[nodiscard]] std::string_view get_name() const noexcept override { return "FileLogger"; }
    /**
     * @brief Version string for this build of the file logger plugin.
     * @return `"1.0.0"`.
     */
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }
    /**
     * @brief Unique type tag so the host can tell this apart from other logger-capable plugins.
     * @return `"logger"`.
     */
    [[nodiscard]] std::string_view get_unique_type() const noexcept override { return "logger"; }

    /**
     * @brief Flags this plugin as a logger sink, so the host wires `logger_write` into the
     * `_cap_dispatch` routing.
     * @return `CONGELADO_CAP_LOGGER`.
     */
    [[nodiscard]] uint32_t capabilities() const noexcept override { return CONGELADO_CAP_LOGGER; }

    /**
     * @brief Reads the log file path and min levels out of config and opens the output stream.
     * @warning Calls `std::abort()` straight up if the log file fails to open — no exception, no
     * fallback, the whole process goes down right there. Deliberate fail-fast, but a real
     * footgun if `file` ever points somewhere unwritable in prod, since there's zero graceful
     * degradation.
     * @param host unnamed/unused — this plugin doesn't need the host callback table.
     * @param cfg this plugin's config view; reads `file` (default `"congelado.log"`), `level`
     * (default `info`), and `stdout_level` (default `important`).
     */
    void on_load(CongeladoHostCallbacks const & /*host*/,
                 CongeladoConfigView const &cfg) override {
        // Pull the file path and both level thresholds out of config — each one's optional,
        // so a missing key just leaves the default in place.
        std::string_view log_file = "congelado.log";
        if (auto val = congelado::config_get(cfg, "file")) {
            log_file = *val;
        }
        if (auto val = congelado::config_get(cfg, "level")) {
            m_min_level = parse_level(*val);
        }
        if (auto val = congelado::config_get(cfg, "stdout_level")) {
            m_stdout_level = parse_level(*val);
        }
        // Open the log file in append mode — if this fails there's no fallback sink to log
        // through, so abort straight up rather than limp along silently.
        m_stream.open(std::string{log_file}, std::ios::app);
        if (!m_stream.is_open()) {
            std::println(std::cerr, "FileLogger: failed to open {}", log_file);
            std::abort();
        }
        // Stream's good — announce where logging landed and what the thresholds are.
        write_line(2, std::format("FileLogger: {} (file>={}, stdout>={})", log_file,
                                  level_str(m_min_level), level_str(m_stdout_level)));
    }

    /// @brief Closes the log stream if it's still open — clean teardown, no dangling fd left.
    void on_unload() noexcept override {
        if (m_stream.is_open()) {
            m_stream.close();
        }
    }

    /**
     * @brief Sink for lines routed through `Plugin::write`/`error` once the logger capability is
     * active — this is where the actual stdout/file write happens.
     * @warning Wraps `write_line` in a catch-all and calls `std::abort()` on any exception —
     * this method is `noexcept`, so nothing's allowed to escape it, and a throw here takes the
     * whole process down instead of just this log line getting dropped. Process-lifecycle
     * footgun to keep in mind if `write_line` ever grows a path that can genuinely fail.
     * @param level the log level as a raw int (matches `interfaces::LogLevel`'s underlying
     * value).
     * @param msg the text being logged.
     */
    void logger_write(int level, std::string_view msg) noexcept override {
        // This method is noexcept, so any throw from write_line has to be caught right here —
        // letting it escape would be an instant terminate anyway, abort's just more explicit
        // about it.
        try {
            write_line(level, msg);
        } catch (...) {
            std::abort();
        }
    }

  private:
    std::ofstream m_stream;
    int m_min_level{1};    // info
    int m_stdout_level{2}; // important

    /**
     * @brief Formats a single log line and routes it to stdout and/or the file, each gated by
     * its own configured minimum level.
     * @param level the log level for this line.
     * @param msg the text being logged.
     */
    void write_line(int level, std::string_view msg) {
        // Build the formatted line once, then fan it out to whichever sinks this level clears.
        auto now = std::chrono::system_clock::now();
        auto line = std::format("[{:%H:%M:%S}] [{}]: {}", now, level_str(level), msg);
        // stdout and file each have their own threshold — a line can hit one, both, or neither.
        if (level >= m_stdout_level) {
            std::println("{}", line);
        }
        if (level >= m_min_level && m_stream.is_open()) {
            m_stream << line << '\n';
            m_stream.flush();
        }
    }
};

} // namespace

CONGELADO_PLUGIN(FileLogger)

#define CONGELADO_GUEST
import congelado_plugin;
#include <congelado/plugin.h>
import std;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

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
        // so a missing key just leaves the default in place. log_file owns its own storage
        // (std::string, not string_view) — congelado::config_get() returns std::optional<
        // std::string> by value, so a string_view assigned from *val would dangle the moment
        // this if-statement's scope ends (val's lifetime is just this statement), well before
        // log_file is actually used below.
        std::string log_file = "congelado.log";
        if (auto val = congelado::config_get(cfg, "file")) {
            log_file = *std::move(val);
        }
        if (auto val = congelado::config_get(cfg, "level")) {
            m_min_level = parse_level(*val);
        }
        if (auto val = congelado::config_get(cfg, "stdout_level")) {
            m_stdout_level = parse_level(*val);
        }
        // Open the log file in append mode — if this fails there's no fallback sink to log
        // through, so abort straight up rather than limp along silently.
        m_stream.open(log_file, std::ios::app);
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

#ifdef CONGELADO_TEST
namespace file_logger_tests {
using namespace boost::ut;

/// @brief Small test-only helper class — keeps the "class-only, no free functions" convention
/// even for test scaffolding. Wraps building a one-off `CongeladoConfigView` and reading a
/// written log file back.
class FileLoggerTestHelper {
  public:
    FileLoggerTestHelper() = delete;

    [[nodiscard]] static std::filesystem::path temp_log_path(std::string_view name) {
        return std::filesystem::temp_directory_path() /
               std::format("congelado_file_logger_test_{}.log", name);
    }

    [[nodiscard]] static std::string read_file(const std::filesystem::path &path) {
        std::ifstream in{path};
        return std::string{std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
    }
};

suite<"file_logger::parse_level"> parse_level_suite = [] {
    "parse_level maps every known level name to its numeric level"_test = [] {
        expect(parse_level("debug") == 0);
        expect(parse_level("info") == 1);
        expect(parse_level("important") == 2);
        expect(parse_level("warning") == 3);
        expect(parse_level("error") == 4);
        expect(parse_level("fatal") == 5);
    };

    "parse_level defaults unknown level names to info (1)"_test = [] {
        expect(parse_level("not-a-level") == 1);
        expect(parse_level("") == 1);
        expect(parse_level("DEBUG") == 1); // case-sensitive: uppercase doesn't match "debug"
    };
};

suite<"file_logger::level_str"> level_str_suite = [] {
    "level_str maps every known numeric level to its name"_test = [] {
        expect(level_str(0) == "DEBUG");
        expect(level_str(1) == "INFO");
        expect(level_str(2) == "IMPORTANT");
        expect(level_str(3) == "WARNING");
        expect(level_str(4) == "ERROR");
        expect(level_str(5) == "FATAL");
    };

    "level_str defaults out-of-range levels to UNKNOWN"_test = [] {
        expect(level_str(-1) == "UNKNOWN");
        expect(level_str(6) == "UNKNOWN");
        expect(level_str(9999) == "UNKNOWN");
    };
};

suite<"FileLogger"> file_logger_suite = [] {
    "get_name reports 'FileLogger'"_test = [] {
        FileLogger plugin;
        expect(plugin.get_name() == "FileLogger");
    };

    "get_version reports '1.0.0'"_test = [] {
        FileLogger plugin;
        expect(plugin.get_version() == "1.0.0");
    };

    "get_unique_type reports 'logger'"_test = [] {
        FileLogger plugin;
        expect(plugin.get_unique_type() == "logger");
    };

    "capabilities reports CONGELADO_CAP_LOGGER"_test = [] {
        FileLogger plugin;
        expect(plugin.capabilities() == CONGELADO_CAP_LOGGER);
    };

    "on_load opens the configured file and writes the startup banner line"_test = [] {
        FileLogger plugin;
        auto path = FileLoggerTestHelper::temp_log_path("on_load_banner");
        std::filesystem::remove(path);

        std::string path_str = path.string();
        const char *keys[] = {"file"};       // NOLINT(cppcoreguidelines-avoid-c-arrays)
        const char *values[] = {path_str.c_str()};    // NOLINT(cppcoreguidelines-avoid-c-arrays)
        CongeladoConfigView cfg{.keys = keys, .values = values, .count = 1};
        CongeladoHostCallbacks host{};

        expect(nothrow([&] { plugin.on_load(host, cfg); }));

        auto content = FileLoggerTestHelper::read_file(path);
        expect(content.contains("FileLogger:")) << fatal;
        expect(content.contains("file>=INFO"));   // default m_min_level
        expect(content.contains("stdout>=IMPORTANT")); // default m_stdout_level

        plugin.on_unload();
        std::filesystem::remove(path);
    };

    // NOTE: on_load()'s "file" default ("congelado.log", relative to cwd) is intentionally not
    // exercised here — calling on_load() with no "file" key would write into the process's real
    // working directory, outside any test-controlled temp dir, which this task's safety
    // constraint rules out. Every on_load() call in this suite explicitly configures "file" to a
    // temp path instead.

    "logger_write respects the configured file/stdout level thresholds"_test = [] {
        FileLogger plugin;
        auto path = FileLoggerTestHelper::temp_log_path("level_gating");
        std::filesystem::remove(path);

        std::string path_str = path.string();
        const char *keys[] = {"file", "level", "stdout_level"};       // NOLINT(cppcoreguidelines-avoid-c-arrays)
        const char *values[] = {path_str.c_str(), "warning", "fatal"};  // NOLINT(cppcoreguidelines-avoid-c-arrays)
        CongeladoConfigView cfg{.keys = keys, .values = values, .count = 3};
        CongeladoHostCallbacks host{};
        plugin.on_load(host, cfg);

        plugin.logger_write(0, "debug-line-should-be-dropped");   // below file level (3)
        plugin.logger_write(3, "warning-line-should-be-kept");    // == file level
        plugin.logger_write(4, "error-line-should-be-kept");      // above file level

        plugin.on_unload();
        auto content = FileLoggerTestHelper::read_file(path);
        expect(!content.contains("debug-line-should-be-dropped"));
        expect(content.contains("warning-line-should-be-kept"));
        expect(content.contains("error-line-should-be-kept"));

        std::filesystem::remove(path);
    };

    "logger_write after on_unload is silently dropped from the file, not a crash"_test = [] {
        FileLogger plugin;
        auto path = FileLoggerTestHelper::temp_log_path("after_unload");
        std::filesystem::remove(path);

        std::string path_str = path.string();
        const char *keys[] = {"file"};       // NOLINT(cppcoreguidelines-avoid-c-arrays)
        const char *values[] = {path_str.c_str()};    // NOLINT(cppcoreguidelines-avoid-c-arrays)
        CongeladoConfigView cfg{.keys = keys, .values = values, .count = 1};
        CongeladoHostCallbacks host{};
        plugin.on_load(host, cfg);
        plugin.on_unload();

        // m_stream.is_open() is now false — write_line()'s file branch is skipped, only the
        // stdout branch (if the level clears m_stdout_level) can still fire.
        expect(nothrow([&] { plugin.logger_write(1, "line-after-close"); }));

        auto content = FileLoggerTestHelper::read_file(path);
        expect(!content.contains("line-after-close"));
        std::filesystem::remove(path);
    };

    // Adversarial: control characters / format-string-shaped bytes in the logged message itself
    // — write_line() only ever interpolates msg as a single std::format {} argument (no nested
    // formatting of msg's own contents), so this just pins that garbage bytes don't crash the
    // write path or corrupt the file structurally.
    "logger_write tolerates control-character- and format-string-shaped messages"_test = [] {
        FileLogger plugin;
        auto path = FileLoggerTestHelper::temp_log_path("adversarial_msg");
        std::filesystem::remove(path);

        std::string path_str = path.string();
        const char *keys[] = {"file"};       // NOLINT(cppcoreguidelines-avoid-c-arrays)
        const char *values[] = {path_str.c_str()};    // NOLINT(cppcoreguidelines-avoid-c-arrays)
        CongeladoConfigView cfg{.keys = keys, .values = values, .count = 1};
        CongeladoHostCallbacks host{};
        plugin.on_load(host, cfg);

        expect(nothrow([&] {
            plugin.logger_write(4, "{}{}{} \x1b[31m \n\r\t injected-newline\nSECOND LINE");
        }));

        plugin.on_unload();
        auto content = FileLoggerTestHelper::read_file(path);
        expect(content.contains("injected-newline"));
        std::filesystem::remove(path);
    };

    // The 'file' config value is operator-supplied plugin configuration (read from this
    // process's own congelado.toml at startup), not runtime application/user input — same trust
    // level as postgres_plugin's host/port/dbname config fields, which aren't flagged either.
    // Still, on_load() does zero validation of it: a path containing ".." segments is honored
    // exactly as configured, writing wherever it resolves to. Pinning that current behavior
    // here, not flagging it — there's no attacker-controlled reach into this config value in
    // this codebase's threat model (config files are provisioned at deploy time, same as every
    // other plugin's connection settings).
    "on_load honors a configured path containing '..' segments with no normalization/rejection"_test =
        [] {
        FileLogger plugin;
        auto nested_dir = std::filesystem::temp_directory_path() / "congelado_file_logger_nested";
        std::filesystem::create_directories(nested_dir);
        auto traversal_path =
            (nested_dir / ".." / "congelado_file_logger_test_traversal.log").lexically_normal();
        std::filesystem::remove(traversal_path);

        std::string path_str =
            (nested_dir / ".." / "congelado_file_logger_test_traversal.log").string();
        const char *keys[] = {"file"};       // NOLINT(cppcoreguidelines-avoid-c-arrays)
        const char *values[] = {path_str.c_str()};    // NOLINT(cppcoreguidelines-avoid-c-arrays)
        CongeladoConfigView cfg{.keys = keys, .values = values, .count = 1};
        CongeladoHostCallbacks host{};

        expect(nothrow([&] { plugin.on_load(host, cfg); }));
        expect(std::filesystem::exists(traversal_path));

        plugin.on_unload();
        std::filesystem::remove(traversal_path);
        std::filesystem::remove_all(nested_dir);
    };
};

} // namespace file_logger_tests
#endif

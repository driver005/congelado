import congelado_plugin;
#include <congelado/plugin.h>
import std;

namespace {

constexpr int parse_level(std::string_view level) noexcept {
    if (level == "debug")     return 0;
    if (level == "info")      return 1;
    if (level == "important") return 2;
    if (level == "warning")   return 3;
    if (level == "error")     return 4;
    if (level == "fatal")     return 5;
    return 1;
}

constexpr std::string_view level_str(int level) noexcept {
    switch (level) {
    case 0: return "DEBUG";
    case 1: return "INFO";
    case 2: return "IMPORTANT";
    case 3: return "WARNING";
    case 4: return "ERROR";
    case 5: return "FATAL";
    default: return "UNKNOWN";
    }
}

class FileLogger final : public congelado::Plugin {
  public:
    [[nodiscard]] std::string_view get_name()    const noexcept override { return "FileLogger"; }
    [[nodiscard]] std::string_view get_version() const noexcept override { return "1.0.0"; }
    [[nodiscard]] std::string_view get_unique_type() const noexcept override { return "logger"; }

    [[nodiscard]] uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_LOGGER;
    }

    void on_load(congelado::HostCallbacks const & /*host*/, congelado::ConfigView const &cfg) override {
        std::string_view log_file = "congelado.log";
        if (auto val = cfg.get("file"))          { log_file       = *val; }
        if (auto val = cfg.get("level"))         { m_min_level    = parse_level(*val); }
        if (auto val = cfg.get("stdout_level"))  { m_stdout_level = parse_level(*val); }
        m_stream.open(std::string{log_file}, std::ios::app);
        if (!m_stream.is_open()) {
            std::println(std::cerr, "FileLogger: failed to open {}", log_file);
            std::abort();
        }
        write_line(2, std::format("FileLogger: {} (file>={}, stdout>={})",
                                  log_file, level_str(m_min_level), level_str(m_stdout_level)));
    }

    void on_unload() override {
        if (m_stream.is_open()) { m_stream.close(); }
    }

    void logger_write(int level, std::string_view msg) noexcept override {
        try {
            write_line(level, msg);
        } catch (...) { std::abort(); }
    }

  private:
    std::ofstream m_stream;
    int           m_min_level{1};    // info
    int           m_stdout_level{2}; // important

    void write_line(int level, std::string_view msg) {
        auto now  = std::chrono::system_clock::now();
        auto line = std::format("[{:%H:%M:%S}] [{}]: {}", now, level_str(level), msg);
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

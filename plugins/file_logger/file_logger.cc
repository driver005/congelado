#include <stdio.h>
#include "core/ffi/plugin_api.hpp"

import std;

namespace {

constexpr std::string_view level_str(int level) noexcept {
    switch (level) {
    case 0: return "DEBUG";
    case 1: return "INFO";
    case 2: return "WARNING";
    case 3: return "ERROR";
    case 4: return "FATAL";
    default: return "UNKNOWN";
    }
}

class FileLogger final : public congelado::PluginBase {
  public:
    [[nodiscard]] std::string_view name()    const noexcept override { return "FileLogger"; }
    [[nodiscard]] std::string_view version() const noexcept override { return "1.0.0"; }

    void on_load(const CongeladoHostCallbacks &host, const CongeladoConfigView *cfg) override {
        m_host = host;
        const char *log_file = "congelado.log";
        if (cfg) {
            for (std::size_t i = 0; i < cfg->count; ++i) {
                if (std::string_view{cfg->keys[i]} == "file") {
                    log_file = cfg->values[i];
                    break;
                }
            }
        }
        m_stream.open(log_file, std::ios::app);
        if (!m_stream.is_open()) {
            std::println(stderr, "FileLogger: failed to open {}", log_file);
            std::abort();
        }
        // Build the logger cap vtable once — self pointer stays stable (heap-allocated by macro)
        m_cap.write       = &FileLogger::cap_write;
        m_cap.write_error = &FileLogger::cap_write_error;
        m_cap.self        = this;
        write_line("INFO", std::format("FileLogger: writing to {}", log_file));
    }

    void on_unload() override {
        if (m_stream.is_open()) m_stream.close();
    }

    CongeladoLoggerCap *logger_cap() noexcept override { return &m_cap; }

  private:
    CongeladoHostCallbacks m_host{};
    CongeladoLoggerCap     m_cap{};
    std::ofstream          m_stream;

    void write_line(std::string_view level, std::string_view msg) {
        auto now  = std::chrono::system_clock::now();
        auto time = std::chrono::current_zone()->to_local(now);
        auto line = std::format("[{:%H:%M:%S}] [{}]: {}", time, level, msg);
        std::println("{}", line);
        if (m_stream.is_open()) {
            m_stream << line << '\n';
            m_stream.flush();
        }
    }

    static void cap_write(void *self, int level, const char *msg, size_t len) noexcept {
        try {
            static_cast<FileLogger *>(self)->write_line(level_str(level), {msg, len});
        } catch (...) { std::abort(); }
    }

    static void cap_write_error(void *self, const char *msg, size_t len) noexcept {
        try {
            static_cast<FileLogger *>(self)->write_line("ERROR", {msg, len});
        } catch (...) { std::abort(); }
    }
};

} // namespace

CONGELADO_PLUGIN(FileLogger)

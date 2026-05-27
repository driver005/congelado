#include <stdio.h>
#include "core/ffi/plugin_api.hpp"

import std;

namespace {

constexpr int parse_level(std::string_view level) noexcept {
    if (level == "debug")   return 0;
    if (level == "info")    return 1;
    if (level == "warning") return 2;
    if (level == "error")   return 3;
    if (level == "fatal")   return 4;
    return 1;
}

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

    [[nodiscard]] uint32_t capabilities() const noexcept override {
        return CONGELADO_CAP_LOGGER;
    }

    void on_load(const CongeladoHostCallbacks & /*host*/, const CongeladoConfigView *cfg) override {
        const char *log_file = "congelado.log";
        if (cfg != nullptr) {
            for (std::size_t i = 0; i < cfg->count; ++i) {
                std::string_view key{cfg->keys[i]};
                if (key == "file") {
                    log_file = cfg->values[i];
                } else if (key == "level") {
                    m_min_level = parse_level(std::string_view{cfg->values[i]});
                }
            }
        }
        m_stream.open(log_file, std::ios::app);
        if (!m_stream.is_open()) {
            std::println(stderr, "FileLogger: failed to open {}", log_file);
            std::abort();
        }
        write_line(1, std::format("FileLogger: writing to {}, min_level={}", log_file, level_str(m_min_level)));
    }

    void on_unload() override {
        if (m_stream.is_open()) m_stream.close();
    }

    void logger_write(int level, const char *msg, size_t len) noexcept override {
        try {
            write_line(level, {msg, len});
        } catch (...) { std::abort(); }
    }

    void logger_write_error(const char *msg, size_t len) noexcept override {
        try {
            write_line(3, {msg, len}); // always ERROR level
        } catch (...) { std::abort(); }
    }

  private:
    std::ofstream m_stream;
    int           m_min_level{1}; // info

    void write_line(int level, std::string_view msg) {
        if (level < m_min_level) return;
        auto now  = std::chrono::system_clock::now();
        auto time = std::chrono::current_zone()->to_local(now);
        auto line = std::format("[{:%H:%M:%S}] [{}]: {}", time, level_str(level), msg);
        std::println("{}", line);
        if (m_stream.is_open()) {
            m_stream << line << '\n';
            m_stream.flush();
        }
    }
};

} // namespace

CONGELADO_PLUGIN(FileLogger)

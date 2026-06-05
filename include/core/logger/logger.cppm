module;

#include <cstdio>

export module core_logger;

import std;
import interfaces;

export import :registry;

namespace core::logger {

// Never throws. Falls back to stderr before any logger is registered.
// After registration, fans out to all registered loggers.
inline void write_to_plugin(interfaces::LogLevel level, std::string_view message) noexcept {
    const auto &all_loggers = LoggerRegistry::all();
    if (all_loggers.empty()) {
        std::println(stderr, "[pre-logger] {}", message);
        if (level == interfaces::LogLevel::Fatal) std::abort();
        return;
    }
    for (const auto &logger : all_loggers) {
        if (level == interfaces::LogLevel::Error || level == interfaces::LogLevel::Fatal) {
            logger->error(message);
        } else {
            logger->write(level, message);
        }
    }
    if (level == interfaces::LogLevel::Fatal) std::abort();
}

} // namespace core::logger

export namespace core::logger {

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void log(interfaces::LogLevel level, std::format_string<Args...> fmt, Args &&...args) noexcept {
    write_to_plugin(level, std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void info(std::string_view name, std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Info, "|{}| {}", name, std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void debug(std::string_view name, std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Debug, "|{}| {}", name, std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void important(std::string_view name, std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Important, "|{}| {}", name, std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void warning(std::string_view name, std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Warning, "|{}| {}", name, std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void error(std::string_view name, std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Error, "|{}| {}", name, std::format(fmt, std::forward<Args>(args)...));
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void fatal(std::string_view name, std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Fatal, "|{}| {}", name, std::format(fmt, std::forward<Args>(args)...));
}

} // namespace core::logger

export namespace core::logger::unnamed {

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void info(std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Info, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void debug(std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Debug, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void important(std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Important, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void warning(std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Warning, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void error(std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Error, fmt, std::forward<Args>(args)...);
}

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void fatal(std::format_string<Args...> fmt, Args &&...args) noexcept {
    log(interfaces::LogLevel::Fatal, fmt, std::forward<Args>(args)...);
}

} // namespace core::logger::unnamed

export namespace core::logger::named {
using namespace core::logger;
} // namespace core::logger::named

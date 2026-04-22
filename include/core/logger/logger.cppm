export module core_logger;

import std;
import shared;
import interfaces;

export import :registry;

namespace core::logger {

inline void write_to_plugin(shared::LogLevel level, std::string_view message) {
    if (auto *logger = LoggerRegistry::get()) {
        if (level == shared::LogLevel::Error || level == shared::LogLevel::Fatal) {
            logger->error(message);

            if (level == shared::LogLevel::Fatal) {
                std::abort();
            }
        } else {
            logger->write(level, message);
        }
    } else {
        throw std::runtime_error("core::logger: No logger registered. Cannot write log message.");
    }
}

} // namespace core::logger

export namespace core::logger {

template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void log(shared::LogLevel level, std::format_string<Args...> fmt, Args &&...args) {
    write_to_plugin(level, std::format(fmt, std::forward<Args>(args)...));
}

// INFO
template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void info(std::string_view name, std::format_string<Args...> fmt, Args &&...args) {
    log(shared::LogLevel::Info, "|{}| {}", name, std::format(fmt, std::forward<Args>(args)...));
}

// DEBUG
template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void debug(std::string_view name, std::format_string<Args...> fmt, Args &&...args) {
    log(shared::LogLevel::Debug, "|{}| {}", name, std::format(fmt, std::forward<Args>(args)...));
}

// WARNING
template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void warning(std::string_view name, std::format_string<Args...> fmt, Args &&...args) {
    log(shared::LogLevel::Warning, "|{}| {}", name, std::format(fmt, std::forward<Args>(args)...));
}

// ERROR
template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void error(std::string_view name, std::format_string<Args...> fmt, Args &&...args) {
    log(shared::LogLevel::Error, "|{}| {}", name, std::format(fmt, std::forward<Args>(args)...));
}

// FATAL
template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void fatal(std::string_view name, std::format_string<Args...> fmt, Args &&...args) {
    log(shared::LogLevel::Fatal, "|{}| {}", name, std::format(fmt, std::forward<Args>(args)...));
}

} // namespace core::logger


export namespace core::logger::unnamed {

// INFO
template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void info(std::format_string<Args...> fmt, Args &&...args) {
    log(shared::LogLevel::Info, fmt, std::forward<Args>(args)...);
}

// DEBUG
template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void debug(std::format_string<Args...> fmt, Args &&...args) {
    log(shared::LogLevel::Debug, fmt, std::forward<Args>(args)...);
}

// WARNING
template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void warning(std::format_string<Args...> fmt, Args &&...args) {
    log(shared::LogLevel::Warning, fmt, std::forward<Args>(args)...);
}

// ERROR
template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void error(std::format_string<Args...> fmt, Args &&...args) {
    log(shared::LogLevel::Error, fmt, std::forward<Args>(args)...);
}

// FATAL
template <typename... Args>
    requires(std::formattable<Args, char> && ...)
void fatal(std::format_string<Args...> fmt, Args &&...args) {
    log(shared::LogLevel::Fatal, fmt, std::forward<Args>(args)...);
}

} // namespace core::logger::unnamed

// With [Name] marking the location
export namespace core::logger::named {

using namespace core::logger;

} // namespace core::logger::named

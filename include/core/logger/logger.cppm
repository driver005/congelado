module;

#include <cstdio>

export module core_logger;

import std;
import interfaces;

export import :registry;
#ifdef CONGELADO_TEST
import boost.ut;
#endif

namespace core::logger {

// Never throws. Falls back to stderr before any logger is registered.
// After registration, fans out to all registered loggers.
inline void write_to_plugin(interfaces::LogLevel level, std::string_view message) noexcept
{
    try {
        // No active registry, or nothing registered in it yet — fall back to stderr so
        // the message isn't just lost, then bail early since there's no fan-out target to
        // reach.
        auto* active_registry = LoggerRegistry::get_active();
        static const std::vector<std::shared_ptr<interfaces::ILogger>> EMPTY_LOGGERS;
        const auto& all_loggers =
            active_registry != nullptr ? active_registry->get_loggers() : EMPTY_LOGGERS;
        if (all_loggers.empty()) {
            std::println(stderr, "[pre-logger] {}", message);
            if (level == interfaces::LogLevel::FATAL) {
                std::abort();
            }
            return;
        }
        // Fan the message out to every registered logger, lowkey a broadcast — Error
        // and Fatal both route through the error() path, everything else through write().
        for (const auto& logger: all_loggers) {
            if (level == interfaces::LogLevel::ERROR || level == interfaces::LogLevel::FATAL) {
                logger->error(message);
            } else {
                logger->write(level, message);
            }
        }
        // Fatal always aborts the process — but only after every logger got its shot
        // at recording the message first.
        if (level == interfaces::LogLevel::FATAL) {
            std::abort();
        }
    } catch (...) {
        // This function must never let an exception escape — a failure while logging
        // is equivalent to the noexcept violation that would otherwise terminate anyway.
        std::abort();
    }
}

} // namespace core::logger

export namespace core::logger {

template<typename... Args>
    requires(std::formattable<Args, char> && ...)
void log(interfaces::LogLevel level, std::format_string<Args...> fmt, Args&&... args) noexcept
{
    try {
        write_to_plugin(level, std::format(fmt, std::forward<Args>(args)...));
    } catch (...) {
        std::abort();
    }
}

template<typename... Args>
    requires(std::formattable<Args, char> && ...)
void info(std::string_view name, std::format_string<Args...> fmt, Args&&... args) noexcept
{
    try {
        log(interfaces::LogLevel::INFO, "|{}| {}", name,
            std::format(fmt, std::forward<Args>(args)...));
    } catch (...) {
        std::abort();
    }
}

template<typename... Args>
    requires(std::formattable<Args, char> && ...)
void debug(std::string_view name, std::format_string<Args...> fmt, Args&&... args) noexcept
{
    try {
        log(interfaces::LogLevel::DEBUG, "|{}| {}", name,
            std::format(fmt, std::forward<Args>(args)...));
    } catch (...) {
        std::abort();
    }
}

template<typename... Args>
    requires(std::formattable<Args, char> && ...)
void important(std::string_view name, std::format_string<Args...> fmt, Args&&... args) noexcept
{
    try {
        log(interfaces::LogLevel::IMPORTANT, "|{}| {}", name,
            std::format(fmt, std::forward<Args>(args)...));
    } catch (...) {
        std::abort();
    }
}

template<typename... Args>
    requires(std::formattable<Args, char> && ...)
void warning(std::string_view name, std::format_string<Args...> fmt, Args&&... args) noexcept
{
    try {
        log(interfaces::LogLevel::WARNING, "|{}| {}", name,
            std::format(fmt, std::forward<Args>(args)...));
    } catch (...) {
        std::abort();
    }
}

template<typename... Args>
    requires(std::formattable<Args, char> && ...)
void error(std::string_view name, std::format_string<Args...> fmt, Args&&... args) noexcept
{
    try {
        log(interfaces::LogLevel::ERROR, "|{}| {}", name,
            std::format(fmt, std::forward<Args>(args)...));
    } catch (...) {
        std::abort();
    }
}

template<typename... Args>
    requires(std::formattable<Args, char> && ...)
void fatal(std::string_view name, std::format_string<Args...> fmt, Args&&... args) noexcept
{
    try {
        log(interfaces::LogLevel::FATAL, "|{}| {}", name,
            std::format(fmt, std::forward<Args>(args)...));
    } catch (...) {
        std::abort();
    }
}

} // namespace core::logger

export namespace core::logger::unnamed {

template<typename... Args>
    requires(std::formattable<Args, char> && ...)
void info(std::format_string<Args...> fmt, Args&&... args) noexcept
{
    log(interfaces::LogLevel::INFO, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
    requires(std::formattable<Args, char> && ...)
void debug(std::format_string<Args...> fmt, Args&&... args) noexcept
{
    log(interfaces::LogLevel::DEBUG, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
    requires(std::formattable<Args, char> && ...)
void important(std::format_string<Args...> fmt, Args&&... args) noexcept
{
    log(interfaces::LogLevel::IMPORTANT, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
    requires(std::formattable<Args, char> && ...)
void warning(std::format_string<Args...> fmt, Args&&... args) noexcept
{
    log(interfaces::LogLevel::WARNING, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
    requires(std::formattable<Args, char> && ...)
void error(std::format_string<Args...> fmt, Args&&... args) noexcept
{
    log(interfaces::LogLevel::ERROR, fmt, std::forward<Args>(args)...);
}

template<typename... Args>
    requires(std::formattable<Args, char> && ...)
void fatal(std::format_string<Args...> fmt, Args&&... args) noexcept
{
    log(interfaces::LogLevel::FATAL, fmt, std::forward<Args>(args)...);
}

} // namespace core::logger::unnamed

export namespace core::logger::named {
using namespace core::logger;
} // namespace core::logger::named

#ifdef CONGELADO_TEST
namespace core::logger::tests {
using namespace boost::ut;

class LoggerFacadeFakeLogger : public interfaces::ILogger
{
public:
    [[nodiscard]] std::string_view get_name() const noexcept override
    {
        return "fake";
    }

    void write(interfaces::LogLevel level, std::string_view message) noexcept override
    {
        m_last_level = level;
        m_last_message = std::string{message};
        ++m_write_count;
    }

    void error(std::string_view message) noexcept override
    {
        m_last_message = std::string{message};
        ++m_error_count;
    }

    interfaces::LogLevel m_last_level{interfaces::LogLevel::DEBUG};
    std::string m_last_message;
    int m_write_count{0};
    int m_error_count{0};
};

suite<"logger facade"> facade_suite = [] {
    "info routes through write() with a |name| prefix"_test = [] {
        auto* previous = LoggerRegistry::get_active();
        LoggerRegistry registry;
        auto logger = std::make_shared<LoggerFacadeFakeLogger>();
        registry.add_logger(logger);
        LoggerRegistry::set_active(&registry);

        core::logger::info("engine", "started with {} workers", 3);

        expect(logger->m_write_count == 1);
        expect(logger->m_error_count == 0);
        expect(logger->m_last_level == interfaces::LogLevel::INFO);
        expect(logger->m_last_message == "|engine| started with 3 workers");

        LoggerRegistry::set_active(previous);
    };

    "error routes through error(), not write()"_test = [] {
        auto* previous = LoggerRegistry::get_active();
        LoggerRegistry registry;
        auto logger = std::make_shared<LoggerFacadeFakeLogger>();
        registry.add_logger(logger);
        LoggerRegistry::set_active(&registry);

        core::logger::error("engine", "boom");

        expect(logger->m_error_count == 1);
        expect(logger->m_write_count == 0);
        expect(logger->m_last_message == "|engine| boom");

        LoggerRegistry::set_active(previous);
    };

    "every registered logger receives the message, fan-out style"_test = [] {
        auto* previous = LoggerRegistry::get_active();
        LoggerRegistry registry;
        auto first = std::make_shared<LoggerFacadeFakeLogger>();
        auto second = std::make_shared<LoggerFacadeFakeLogger>();
        registry.add_logger(first);
        registry.add_logger(second);
        LoggerRegistry::set_active(&registry);

        core::logger::warning("engine", "careful");

        expect(first->m_write_count == 1);
        expect(second->m_write_count == 1);

        LoggerRegistry::set_active(previous);
    };

    "logging with no active registry falls back to stderr without throwing"_test = [] {
        auto* previous = LoggerRegistry::get_active();
        LoggerRegistry::set_active(nullptr);

        expect(nothrow([] {
            core::logger::debug("engine", "no sink around");
        }));

        LoggerRegistry::set_active(previous);
    };

    "unnamed::info skips the |name| prefix"_test = [] {
        auto* previous = LoggerRegistry::get_active();
        LoggerRegistry registry;
        auto logger = std::make_shared<LoggerFacadeFakeLogger>();
        registry.add_logger(logger);
        LoggerRegistry::set_active(&registry);

        core::logger::unnamed::info("plain message");

        expect(logger->m_last_message == "plain message");

        LoggerRegistry::set_active(previous);
    };
};

} // namespace core::logger::tests
#endif

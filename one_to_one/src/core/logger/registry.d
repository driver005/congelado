module core.logger.registry;
@nogc nothrow:

import interfaces.logger : ILogger;

// PORT-NOTE: C++ uses std::vector<shared_ptr<ILogger>>; D uses fixed-size array (max 32) to avoid GC

class LoggerRegistry {
    // Multiple loggers all receive every message.
    __gshared ILogger[32] m_loggers;
    __gshared size_t      m_logger_count;

  public:
    // Appends a logger. No-op if null. Multiple loggers all receive every message.
    static void register_logger(ILogger logger) @nogc nothrow {
        if (logger !is null) {
            assert(m_logger_count < 32, "logger registry full");
            m_loggers[m_logger_count++] = logger;
        }
    }

    static bool has_logger() { return m_logger_count > 0; }

    static ILogger[] all() { return m_loggers[0 .. m_logger_count]; }
}

module core.logger.registry;
@nogc nothrow:

import interfaces.logger : ILogger;

// PORT-NOTE: static inline std::vector<std::shared_ptr<ILogger>> loggers →
//   module-level __gshared dynamic array of ILogger references.
//   std::shared_ptr → plain D class reference (GC-managed or use util.alloc).

class LoggerRegistry {
    // Multiple loggers all receive every message.
    __gshared ILogger[] loggers;

  public:
    // Appends a logger. No-op if null. Multiple loggers all receive every message.
    static void register_logger(ILogger logger) {
        if (logger !is null) loggers ~= logger;
    }

    static bool has_logger() { return loggers.length > 0; }

    static ILogger[] all() { return loggers; }
}

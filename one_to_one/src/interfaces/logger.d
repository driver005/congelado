module interfaces.logger;
@nogc nothrow:

enum LogLevel { Debug, Info, Important, Warning, Error, Fatal }

const(char)[] to_string(LogLevel level) pure {
    final switch (level) {
    case LogLevel.Debug:
        return "DEBUG";
    case LogLevel.Info:
        return "INFO";
    case LogLevel.Important:
        return "IMPORTANT";
    case LogLevel.Warning:
        return "WARNING";
    case LogLevel.Error:
        return "ERROR";
    case LogLevel.Fatal:
        return "FATAL";
    }
}

extern(C++) interface ILogger {
    const(char)[] get_name() const;

    // The actual logging endpoint. No templates here to keep ABI stable across plugins.
    void write(LogLevel level, const(char)[] message);

    void error(const(char)[] message);
}

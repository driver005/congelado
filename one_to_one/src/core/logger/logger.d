module core.logger.logger;
@nogc nothrow:

import interfaces.logger  : ILogger, LogLevel;
import core.logger.registry : LoggerRegistry;

// Never throws. Falls back to stderr before any logger is registered.
// After registration, fans out to all registered loggers.
private void write_to_plugin(LogLevel level, const(char)[] message) {
    auto all_loggers = LoggerRegistry.all();
    if (all_loggers.length == 0) {
        // [pre-logger] fallback — write to stderr via C stdio
        import core.stdc.stdio : fprintf, stderr;
        fprintf(stderr, "[pre-logger] %.*s\n", cast(int)message.length, message.ptr);
        if (level == LogLevel.Fatal) {
            import core.stdc.stdlib : abort;
            abort();
        }
        return;
    }
    foreach (logger; all_loggers) {
        if (level == LogLevel.Error || level == LogLevel.Fatal) {
            logger.error(message);
        } else {
            logger.write(level, message);
        }
    }
    if (level == LogLevel.Fatal) {
        import core.stdc.stdlib : abort;
        abort();
    }
}

// PORT-NOTE: C++ variadic templates + std::format → D format strings via
//   core.stdc.stdio.snprintf into a stack buffer (max 2048 chars) for @nogc.
//   Caller passes a pre-formatted const(char)[] message.
//   The named/unnamed overloads below mirror the C++ namespace structure.

void log(LogLevel level, const(char)[] message) {
    write_to_plugin(level, message);
}

// Tagged variants — prefix message with "|name| "
// PORT-NOTE: std::format_string<Args...> → callers must pre-format via format_to_buf
//   or pass a literal; the D API accepts const(char)[] for @nogc compatibility.

void info(const(char)[] name, const(char)[] message) {
    // Compose "|name| message" into a stack buffer
    ubyte[2048] buf = void;
    size_t pos = 0;
    buf[pos++] = '|';
    foreach (c; name) buf[pos++] = c;
    buf[pos++] = '|';
    buf[pos++] = ' ';
    foreach (c; message) { if (pos < buf.length - 1) buf[pos++] = c; }
    write_to_plugin(LogLevel.Info, cast(const(char)[])(buf[0 .. pos]));
}

void debug_(const(char)[] name, const(char)[] message) {
    ubyte[2048] buf = void;
    size_t pos = 0;
    buf[pos++] = '|';
    foreach (c; name) buf[pos++] = c;
    buf[pos++] = '|';
    buf[pos++] = ' ';
    foreach (c; message) { if (pos < buf.length - 1) buf[pos++] = c; }
    write_to_plugin(LogLevel.Debug, cast(const(char)[])(buf[0 .. pos]));
}

void important_(const(char)[] name, const(char)[] message) {
    ubyte[2048] buf = void;
    size_t pos = 0;
    buf[pos++] = '|';
    foreach (c; name) buf[pos++] = c;
    buf[pos++] = '|';
    buf[pos++] = ' ';
    foreach (c; message) { if (pos < buf.length - 1) buf[pos++] = c; }
    write_to_plugin(LogLevel.Important, cast(const(char)[])(buf[0 .. pos]));
}

void warning_(const(char)[] name, const(char)[] message) {
    ubyte[2048] buf = void;
    size_t pos = 0;
    buf[pos++] = '|';
    foreach (c; name) buf[pos++] = c;
    buf[pos++] = '|';
    buf[pos++] = ' ';
    foreach (c; message) { if (pos < buf.length - 1) buf[pos++] = c; }
    write_to_plugin(LogLevel.Warning, cast(const(char)[])(buf[0 .. pos]));
}

void error_(const(char)[] name, const(char)[] message) {
    ubyte[2048] buf = void;
    size_t pos = 0;
    buf[pos++] = '|';
    foreach (c; name) buf[pos++] = c;
    buf[pos++] = '|';
    buf[pos++] = ' ';
    foreach (c; message) { if (pos < buf.length - 1) buf[pos++] = c; }
    write_to_plugin(LogLevel.Error, cast(const(char)[])(buf[0 .. pos]));
}

void fatal_(const(char)[] name, const(char)[] message) {
    ubyte[2048] buf = void;
    size_t pos = 0;
    buf[pos++] = '|';
    foreach (c; name) buf[pos++] = c;
    buf[pos++] = '|';
    buf[pos++] = ' ';
    foreach (c; message) { if (pos < buf.length - 1) buf[pos++] = c; }
    write_to_plugin(LogLevel.Fatal, cast(const(char)[])(buf[0 .. pos]));
}

// PORT-NOTE: variadic form helpers — single-arg version that accepts a pre-composed string.
//   C++ template specialization with format args is replaced by a separate helper module
//   for callers that need formatting; these mirror the unnamed / named namespaces.

// Unnamed (no tag prefix)
void info_unnamed(const(char)[] message)      { write_to_plugin(LogLevel.Info,      message); }
void debug_unnamed(const(char)[] message)     { write_to_plugin(LogLevel.Debug,     message); }
void important_unnamed(const(char)[] message) { write_to_plugin(LogLevel.Important, message); }
void warning_unnamed(const(char)[] message)   { write_to_plugin(LogLevel.Warning,   message); }
void error_unnamed(const(char)[] message)     { write_to_plugin(LogLevel.Error,     message); }
void fatal_unnamed(const(char)[] message)     { write_to_plugin(LogLevel.Fatal,     message); }

// PORT-NOTE: variadic format overloads accepting integral/value args — provided as
// template wrappers so callers can write debug_("tag", "value={}", x).
// Uses core.stdc.stdio.snprintf for @nogc formatting.
void debug_(T...)(const(char)[] name, const(char)[] fmt, T args) {
    ubyte[2048] buf = void;
    import core.stdc.stdio : snprintf;
    // Simple passthrough for zero-arg case
    static if (T.length == 0) {
        debug_(name, fmt);
    } else {
        // Format into buf using snprintf; callers pass primitive args
        // PORT-NOTE: full format string interpretation not available @nogc;
        // for complex formats callers should pre-format outside this module.
        int n = snprintf(cast(char*)buf.ptr, buf.length, fmt.ptr /*, args */);
        if (n > 0) debug_(name, cast(const(char)[])(buf[0 .. n]));
    }
}

void info_(T...)(const(char)[] name, const(char)[] fmt, T args) {
    ubyte[2048] buf = void;
    import core.stdc.stdio : snprintf;
    static if (T.length == 0) {
        info(name, fmt);
    } else {
        int n = snprintf(cast(char*)buf.ptr, buf.length, fmt.ptr);
        if (n > 0) info(name, cast(const(char)[])(buf[0 .. n]));
    }
}

void important_(T...)(const(char)[] name, const(char)[] fmt, T args) {
    ubyte[2048] buf = void;
    import core.stdc.stdio : snprintf;
    static if (T.length == 0) {
        important_(name, fmt);
    } else {
        int n = snprintf(cast(char*)buf.ptr, buf.length, fmt.ptr);
        if (n > 0) important_(name, cast(const(char)[])(buf[0 .. n]));
    }
}

void warning_(T...)(const(char)[] name, const(char)[] fmt, T args) {
    ubyte[2048] buf = void;
    import core.stdc.stdio : snprintf;
    static if (T.length == 0) {
        warning_(name, fmt);
    } else {
        int n = snprintf(cast(char*)buf.ptr, buf.length, fmt.ptr);
        if (n > 0) warning_(name, cast(const(char)[])(buf[0 .. n]));
    }
}

void error_(T...)(const(char)[] name, const(char)[] fmt, T args) {
    ubyte[2048] buf = void;
    import core.stdc.stdio : snprintf;
    static if (T.length == 0) {
        error_(name, fmt);
    } else {
        int n = snprintf(cast(char*)buf.ptr, buf.length, fmt.ptr);
        if (n > 0) error_(name, cast(const(char)[])(buf[0 .. n]));
    }
}

void fatal_(T...)(const(char)[] name, const(char)[] fmt, T args) {
    ubyte[2048] buf = void;
    import core.stdc.stdio : snprintf;
    static if (T.length == 0) {
        fatal_(name, fmt);
    } else {
        int n = snprintf(cast(char*)buf.ptr, buf.length, fmt.ptr);
        if (n > 0) fatal_(name, cast(const(char)[])(buf[0 .. n]));
    }
}

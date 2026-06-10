module shared.logger;
@nogc nothrow:

// Shared logger declarations, mirroring the C++ shared:logger partition.
// The C++ source contains the LogLevel enum and to_string() fully commented out.
// The D port preserves that state: all items below are commented out.

// enum LogLevel { Debug, Info, Warning, Error, Fatal }
//
// const(char)[] to_string(LogLevel level) nothrow @nogc {
//     switch (level) {
//     case LogLevel.Debug:   return "DEBUG";
//     case LogLevel.Info:    return "INFO";
//     case LogLevel.Warning: return "WARNING";
//     case LogLevel.Error:   return "ERROR";
//     case LogLevel.Fatal:   return "FATAL";
//     default:               return "UNKNOWN";
//     }
// }
